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

namespace Patch
{
	bool Initialise();
	void* SearchSignature(const char* ModuleName, int64_t RVA, const char* Signature, const char* DebugName = nullptr);
}