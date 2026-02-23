#pragma once

#include <cstdint>
#include <iostream>
#include <MinHook.h>
#include "common/utils/log.h"

FORERUNNER_CREATE_LOG_CATEGORY(Patch);

// Create a hook for a function at a certain memory address, arguments are:
// Module the target function is in
// Expected RVA of the function
// Unique signature of the function to scan for if RVA is outdated, i.e. the first few bytes of the function's machine code
// Function name, will be used as the class name of the patch
// Return type
// List of Function parameters
// Ex:
// .h:
// PATCH("game.dll", 0xdeadbeef, "13 37 ?? ?? C0 DE", example_function, void, int param1, float param2)
// .cpp:
// void example_function::Patch(int param1, float param2)
// {
//     Original(param1 + 1, param2);
// }
// Elsewhere:
// example_function::Create();
// [...]
// example_function::Enable();
#define PATCH(Module, ExpectedAddress, Signature, FunctionName, ReturnType, ...) class FunctionName \
{ \
public: \
	static bool Create() \
	{ \
		PATCH_Address = Patch::SearchSignature(Module, PATCH_RVA, PATCH_Signature, #FunctionName); \
		if (!PATCH_Address) return false; \
		MH_STATUS Result = MH_CreateHook(PATCH_Address, &FunctionName::Patch, reinterpret_cast<LPVOID*>(&Original)); \
		if (Result != MH_OK) \
		{ \
			FORERUNNER_ERROR(Patch, "Could not create hook for " Module ": {}", MH_StatusToString(Result)); \
			return false; \
		} \
		return true; \
	} \
	static bool Enable() \
	{ \
		if (!PATCH_Address) return false; \
		MH_STATUS Result = MH_EnableHook(PATCH_Address); \
		if (Result != MH_OK) \
		{ \
			FORERUNNER_ERROR(Patch, "Could not enable hook for " Module ": {}", MH_StatusToString(Result)); \
			return false; \
		} \
		return true; \
	} \
protected: \
	static ReturnType Patch(__VA_ARGS__); \
	static inline ReturnType(*Original)(__VA_ARGS__) = nullptr; \
	static inline LPVOID PATCH_Address = 0; \
	static inline int64_t PATCH_RVA = ExpectedAddress; \
	static inline const char* PATCH_Signature = Signature; \
};

// Define the location of a global variable based on a function call which accesses it, arguments are
// Module the global variable is in
// Expected RVA of the variable
// Unique signature of a segment of code which references this variable
// Offset from the start of the signature to find the RIP-relative address of the global variable
// The type of the global variable
// Variable name which can be used to access the variable once it has been found
// .Find() must be called on the returned Global once before it's value can be accessed
// Ex:
// .h:
// GLOBAL("game.dll", 0xdeadbeef, "13 37 ?? ?? ?? ?? C0 DE", 3, int[512], g_example_global);
// .cpp:
// void Init()
// {
//     g_example_global.Find();
// }
// void Tick()
// {
//    g_example_global[3] = 6;
// }
// somewhere in the game's asm:
// ab cd
// 13 37 [0x1234] // <-Signature start + 3 bytes = 0x1234, the offset from the RIP of the global
// c0 de
// da da
#define GLOBAL(Module, ExpectedAddress, Signature, Offset, Type, Name) static inline Patch::Global<Type> Name = Patch::Global<Type>(Module, ExpectedAddress, Signature, Offset, #Name);
#define OFFSET(Module, ExpectedAddress, Signature, Offset, Name) static inline Patch::MemoryAddress Name = Patch::MemoryAddress(Module, ExpectedAddress, Signature, Offset, #Name);

namespace Patch
{
	bool Initialise();
	void* SearchSignature(const char* ModuleName, int64_t RVA, const char* Signature, const char* DebugName = nullptr);
	void WriteBytes(void* Address, const std::vector<uint8_t>& Bytes);
	void WriteNOPs(void* Address, int Length);
	bool AreThreadsSuspended();
	void SuspendThreads();
	void ResumeThreads();

	template<typename T>
	class Global
	{
	public:
		Global(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: Value(nullptr)
			, Module(InModule)
			, RVA(InRVA)
			, Signature(InSignature)
			, Offset(InOffset)
			, DebugName(InDebugName)
		{
		}

		operator T&()
		{
			return *Value;
		}

		bool Find()
		{
			void* Address = 0;

			// No signature, use the RVA
			if (!Signature || strlen(Signature) == 0)
			{
				Address = Patch::SearchSignature(Module, RVA, Signature, DebugName);
				if (!Address)
				{
					return false;
				}

				// If using the RVA directly don't apply any offsets/dereferences
				Value = reinterpret_cast<T*>(Address);

				return true;
			}

			// Find the memory address of the code which points to this global
			Address = Patch::SearchSignature(Module, 0, Signature, DebugName);
			if (!Address)
			{
				return false;
			}

			// Code typically references globals in the form of mov REG,QWORD PTR [rip+OFFSET]
			// To extract the absolute address from this we need to add the signed 32 bit offset to the 64 bit address of the RIP register
			// RIP register will be pointing to the next instruction to be decoded, i.e. Address + 4
			unsigned char* AdjustedAddress = reinterpret_cast<unsigned char*>(Address) + Offset;
			unsigned char* RIP = AdjustedAddress + 4;
			int32_t RIPOffset = *reinterpret_cast<int32_t*>(AdjustedAddress);

			Value = reinterpret_cast<T*>(RIP + RIPOffset);

			return true;
		}
	protected:
		T* Value = nullptr;
		const char* Module = nullptr;
		int64_t RVA = 0;
		const char* Signature = nullptr;
		int64_t Offset = 0;
		const char* DebugName = nullptr;

	};

	class MemoryAddress
	{
	public:
		MemoryAddress(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: Address(nullptr)
			, Module(InModule)
			, RVA(InRVA)
			, Signature(InSignature)
			, Offset(InOffset)
			, DebugName(InDebugName)
		{
		}

		operator void* ()
		{
			// Return existing address if already located
			if (Address)
			{
				return Address;
			}

			// First call (or previous call failed), look up address
			Address = Patch::SearchSignature(Module, RVA - Offset, Signature, DebugName);
			if (Address)
			{
				// Apply offset to found address, since SearchSignature only finds the start of the signature
				Address = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(Address) + Offset);
			}

			return Address;
		}

	protected:
		void* Address = nullptr;
		const char* Module = nullptr;
		int64_t RVA = 0;
		const char* Signature = nullptr;
		int64_t Offset = 0;
		const char* DebugName = nullptr;
	};
}
