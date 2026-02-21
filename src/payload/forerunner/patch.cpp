#include "patch.h"
#include "common/utils/inject.h"
#include <sstream>
#include <iostream>
#include <vector>
#include <psapi.h>

static bool bHasInitializedMinhook = false;

bool Patch::Initialise()
{
	if (bHasInitializedMinhook)
	{
		return true;
	}

	MH_STATUS InitStatus = MH_Initialize();
	if (InitStatus != MH_OK)
	{
		FORERUNNER_ERROR(Patch, "Minhook failed to initialize : {}", MH_StatusToString(InitStatus));
		return false;
	}

	bHasInitializedMinhook = true;
	return true;
}

void* Patch::SearchSignature(const char* ModuleName, int64_t RVA, const char* Signature, const char* DebugName)
{
	const char* SearchName = DebugName ? DebugName : Signature;

	HMODULE TargetModule = Inject::FindModule(ModuleName);
	if (!TargetModule)
	{
		FORERUNNER_ERROR(Patch, "Failed to find target module {}", ModuleName);
		return nullptr;
	}

	MODULEINFO ModuleInfo;
	BOOL Result = GetModuleInformation(GetCurrentProcess(), TargetModule, &ModuleInfo, sizeof(ModuleInfo));
	if (!Result)
	{
		FORERUNNER_ERROR(Patch, "Failed to get module information for ", ModuleName);
		return nullptr;
	}

	// No signature was provided, no choice but to blindly trust the RVA is correct
	if (!Signature || strlen(Signature) == 0)
	{
		FORERUNNER_WARN(Patch, "No signature provided for {}, assuming RVA {:#08x} is correct", SearchName, RVA);
		return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(ModuleInfo.lpBaseOfDll) + RVA);
	}

	// Convert signature from string to byte array
	std::vector<short> SignatureBytes;
	{
		std::string SignatureString = std::string(Signature);
		std::stringstream SigStream(SignatureString);
		std::string SigByte;
		while (SigStream >> SigByte)
		{
			// Unknown byte, signify with -1
			if (SigByte == "?" || SigByte == "??")
			{
				SignatureBytes.push_back(-1);
			}
			// Hex string to char
			else
			{
				SignatureBytes.push_back(static_cast<short>(strtoul(SigByte.c_str(), NULL, 16)));
			}
		}
	}

	uint8_t* Bytes = reinterpret_cast<uint8_t*>(ModuleInfo.lpBaseOfDll);
	bool bIsOutdated = false;

	// Start by checking RVA
	for (size_t i = 0; i < SignatureBytes.size(); i++)
	{
		// Signature extended past the end of the module
		if (RVA + i >= ModuleInfo.SizeOfImage)
		{
			bIsOutdated = true;
			break;
		}

		// Current byte in signature doesn't match what was found
		if (SignatureBytes[i] != -1 && Bytes[RVA + i] != SignatureBytes[i])
		{
			bIsOutdated = true;
			break;
		}
	}

	// Everything is still valid, use provided address
	if (!bIsOutdated)
	{
		FORERUNNER_LOG(Patch, "{} ({}) successfully located at {:#08x}", SearchName, Signature, RVA);
		return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(ModuleInfo.lpBaseOfDll) + RVA);
	}

	// RVA is outdated, perform a full search of the module's address space looking for the first instance of the signature
	for (DWORD i = 0; i < ModuleInfo.SizeOfImage; i++)
	{
		// Iterate over the whole image and search from current byte until mismatch is found, not very efficient but this only runs a few times at startup
		bool bFound = true;
		for (size_t j = 0; j < SignatureBytes.size(); j++)
		{
			if (SignatureBytes[j] != -1 && Bytes[i + j] != SignatureBytes[j])
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			FORERUNNER_LOG(Patch, "{} ({}) successfuly located at new RVA of {:#08x}", SearchName, Signature, RVA);
			return reinterpret_cast<unsigned char*>(ModuleInfo.lpBaseOfDll) + i;
		}
	}

	FORERUNNER_ERROR(Patch, "Failed to locate {} ({})", SearchName, Signature);
	return nullptr;
}
