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
// 
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
	static bool Disable() \
	{ \
		if (!PATCH_Address) return false; \
		MH_STATUS Result = MH_DisableHook(PATCH_Address); \
		if (Result != MH_OK) \
		{ \
			FORERUNNER_ERROR(Patch, "Could not disable hook for " Module ": {}", MH_StatusToString(Result)); \
			return false; \
		} \
		return true; \
	} \
	static bool Destroy() \
	{ \
		if (!PATCH_Address) return false; \
		MH_STATUS Result = MH_RemoveHook(PATCH_Address); \
		PATCH_Address = nullptr; \
		if (Result != MH_OK) \
		{ \
			FORERUNNER_ERROR(Patch, "Could not remove hook for " Module ": {}", MH_StatusToString(Result)); \
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
// The type of the global variable
// Variable name which can be used to access the variable once it has been found
// Module the global variable is in
// Expected RVA of the variable
// Unique signature of a segment of code which references this variable
// Offset from the start of the signature to find the RIP-relative address of the global variable
// 
// .Find() must be called on the returned Global once before it's value can be accessed
// Ex:
// .h:
// GLOBAL(int[512], g_example_global, "game.dll", 0xdeadbeef, "13 37 ?? ?? ?? ?? C0 DE", 3);
// .cpp:
// void Init()
// {
//     g_example_global.Find();
// }
// void Tick()
// {
//    g_example_global()[3] = 6;
// }
// somewhere in the game's asm:
// ab cd
// 13 37 [0x1234] // <-Signature start + 3 bytes = 0x1234, the offset from the RIP of the global
// c0 de
// da da
#define GLOBAL(Type, Name, Module, ExpectedAddress, Signature, Offset) inline Patch::Global<Type> Name = Patch::Global<Type>(Module, ExpectedAddress, Signature, Offset, #Name);
// Define the location of a code fragment relative to a known pattern of bytes (signature)
// First access to the offset will automatically locate it in memory, 
// as it is assumed these will only be used during start up to patch functions directly
// Arguments are:
// Variable name used to access the memory address
// Module the code is in
// Expected RVA of the code
// Unique signature of a segment of code close to the target address
// Offset in bytes from the start of the signature to the target address
// 
// Ex:
// .h:
// OFFSET(draw_cinematic_bars, "game.dll", 0xdeadbeef, "AA BB CC DD ?? EE", +8);
// .cpp
// WriteBytes(draw_cinematic_bars, {0x90, 0x90, 0x90, 0x90, 0x90});
#define OFFSET(Name, Module, ExpectedAddress, Signature, Offset) inline Patch::MemoryAddress Name = Patch::MemoryAddress(Module, ExpectedAddress, Signature, Offset, #Name);
// Define a function pointer to a function in the game's code, arguments are:
// Module the function is in
// Expected RVA of the function
// Unique signature of code from within that function
// Offset from the start of the signature to find the start of the function
// Return type of the function
// Function name
// Arguments of the function
// 
// Ex:
// .h
// FUNCTION("game.dll", 0xdeadbeef, "AA BB CC DD", -4, bool game_is_player_valid, int player_id, int controller_id);
// .cpp:
// void Init()
// {
//     game_is_player_valid.Find();
// }
// 
// void Tick()
// {
//     bool IsValid = game_is_player_valid(0, 1);
// }
// game source (probably):
// bool game_is_player_valid(int player_id, int controller_id);
//
#define FUNCTION(Module, ExpectedAddress, Signature, Offset, ReturnType, FunctionName, ...) inline Patch::Function<ReturnType(__VA_ARGS__)> FunctionName = Patch::Function<ReturnType(__VA_ARGS__)>(Module, ExpectedAddress, Signature, Offset, #FunctionName);
// Same as function, only the signature + offset is for a call to this function, rather than the function itself
// (This is sometimes necessary if the target function is too small to uniquely identify via a signature)
#define FUNCTION_CALL(Module, ExpectedAddress, Signature, Offset, ReturnType, FunctionName, ...) inline Patch::FunctionFromCall<ReturnType(__VA_ARGS__)> FunctionName = Patch::FunctionFromCall<ReturnType(__VA_ARGS__)>(Module, ExpectedAddress, Signature, Offset, #FunctionName);

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
	class BaseMemoryPtr
	{
	public:
		BaseMemoryPtr(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: Value(nullptr)
			, Module(InModule)
			, RVA(InRVA)
			, Signature(InSignature)
			, Offset(InOffset)
			, DebugName(InDebugName)
		{
		}

	protected:

		void* SearchMemory(int64_t RVA)
		{
			return Patch::SearchSignature(this->Module, RVA, this->Signature, this->DebugName);
		}

		void* SearchRipRelative()
		{
			void* Address = 0;

			// No signature, use the RVA
			if (!this->Signature || strlen(this->Signature) == 0)
			{
				Address = this->SearchMemory(this->RVA);
				if (!Address)
				{
					return 0;
				}

				// If using the RVA directly don't apply any offsets/dereferences
				return reinterpret_cast<void*>(Address);
			}

			// Find the memory address of the code which points to this global
			Address = this->SearchMemory(0);
			if (!Address)
			{
				return 0;
			}

			// Code typically references globals in the form of mov REG,QWORD PTR [rip+OFFSET]
			// To extract the absolute address from this we need to add the signed 32 bit offset to the 64 bit address of the RIP register
			// RIP register will be pointing to the next instruction to be decoded, i.e. Address + 4
			unsigned char* AdjustedAddress = reinterpret_cast<unsigned char*>(Address) + this->Offset;
			unsigned char* RIP = AdjustedAddress + 4;
			int32_t RIPOffset = *reinterpret_cast<int32_t*>(AdjustedAddress);

			return reinterpret_cast<void*>(RIP + RIPOffset);
		}

		T* Value = nullptr;
		const char* Module = nullptr;
		int64_t RVA = 0;
		const char* Signature = nullptr;
		int64_t Offset = 0;
		const char* DebugName = nullptr;
	};

	template<typename T>
	class Global : public BaseMemoryPtr<T>
	{
	public:
		Global(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: BaseMemoryPtr<T>(InModule, InRVA, InSignature, InOffset, InDebugName)
		{
		}

		// Automatically convert to value where possible
		operator T&()
		{
			return *this->Value;
		}

		// Explicit conversion to value
		T& operator()()
		{
			return *this->Value;
		}

		// Gets a pointer to the global variable, must be called before attempting to access the global
		bool Find()
		{
			this->Value = reinterpret_cast<T*>(this->SearchRipRelative());

			return this->Value != nullptr;
		}
	};

	class MemoryAddress : public BaseMemoryPtr<void>
	{
	public:
		MemoryAddress(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: BaseMemoryPtr<void>(InModule, InRVA, InSignature, InOffset, InDebugName)
		{
		}

		operator void* ()
		{
			// First call (or previous call failed), look up address
			if (!this->Value)
			{
				void* Address = this->SearchMemory(this->RVA - this->Offset);
				if (Address)
				{
					// Apply offset to found address, since SearchSignature only finds the start of the signature
					this->Value = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(Address) + this->Offset);
				}
			}

			return this->Value;
		}
	};

	template<typename T>
	class Function : public BaseMemoryPtr<T>
	{
	public:
		Function(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: BaseMemoryPtr<T>(InModule, InRVA, InSignature, InOffset, InDebugName)
		{
		}

		// Automatically convert to value where possible
		operator T& ()
		{
			return *this->Value;
		}

		// Gets a pointer to the global variable, must be called before attempting to invoke the function
		bool Find()
		{
			void* Address = this->SearchMemory(this->RVA - this->Offset);
			if (!Address)
			{
				return false;
			}

			// Apply offset to found address, since SearchSignature only finds the start of the signature
			this->Value = reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(Address) + this->Offset);
			
			return true;
		}
	};

	template<typename T>
	class FunctionFromCall : public BaseMemoryPtr<T>
	{
	public:
		FunctionFromCall(const char* InModule, int64_t InRVA, const char* InSignature, int64_t InOffset, const char* InDebugName = nullptr)
			: BaseMemoryPtr<T>(InModule, InRVA, InSignature, InOffset, InDebugName)
		{
		}

		// Automatically convert to value where possible
		operator T& ()
		{
			return *this->Value;
		}

		// Gets a pointer to the global variable, must be called before attempting to invoke the function
		bool Find()
		{
			this->Value = reinterpret_cast<T*>(this->SearchRipRelative());

			return this->Value != nullptr;
		}
	};

	template<typename... P>
	struct PatchList
	{
	public:

		static inline bool CreateAll()
		{
			bool bSuccess = true;

			((bSuccess |= P::Create()), ...);

			return bSuccess;
		}
		static inline bool EnableAll()
		{
			bool bSuccess = true;

			((bSuccess |= P::Enable()), ...);

			return bSuccess;
		}
		static inline bool DisableAll()
		{
			bool bSuccess = true;

			((bSuccess |= P::Disable()), ...);

			return bSuccess;
		}
		static inline bool DestroyAll()
		{
			bool bSuccess = true;

			((bSuccess |= P::Destroy()), ...);

			return bSuccess;
		}
	};
}
