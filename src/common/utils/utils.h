#pragma once
#include <filesystem>

template <typename F>
struct ScopeExit
{
	ScopeExit(F f) : f(f)
	{
	}
	~ScopeExit()
	{
		f();
	}
	F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f)
{
	return ScopeExit<F>(f);
};

#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)
#define DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define SCOPE_EXIT(code) \
    auto STRING_JOIN2(scope_exit_, __LINE__) = MakeScopeExit([=](){code;})

constexpr float Deg2Rad(float Degrees)
{
	constexpr float ConversionRatio = 3.1415926535f / 180.0f;
	return Degrees * ConversionRatio;
}

constexpr float Rad2Deg(float Radians)
{
	constexpr float ConversionRatio = 180.0f / 3.1415926535f;
	return Radians * ConversionRatio;
}

inline std::filesystem::path ForerunnerPath;