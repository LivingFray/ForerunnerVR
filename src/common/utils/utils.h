#pragma once
#include <filesystem>
#include "vectors.h"
#include "matrices.h"

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

// Assuming underlying layout is the same types can be converted between module specific types with this helper
template <typename To, typename From>
const To& SameCast(const From& from)
{
	static_assert(std::is_standard_layout_v<From>, "From type must be standard layout");
	static_assert(std::is_standard_layout_v<To>, "To type must be standard layout");
	static_assert(sizeof(To) == sizeof(From), "Types must be the same size");

	return reinterpret_cast<const To&>(from);
}

inline Vector4 Vector4FromPoint(const Vector3& V)
{
	return Vector4(V.x, V.y, V.z, 1.0f);
}

inline Vector4 Vector4FromVector(const Vector3& V)
{
	return Vector4(V.x, V.y, V.z, 0.0f);
}

inline Vector3 Vector3FromVector4(const Vector4& V)
{
	return Vector3(V.x, V.y, V.z);
}


// Formatting
#include <format>
template <>
struct std::formatter<Vector3> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(Vector3 v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
	}
};

template <>
struct std::formatter<Vector4> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(Vector4 v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
	}
};

template <>
struct std::formatter<Matrix3> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(Matrix3 m, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
	}
};

template <>
struct std::formatter<Matrix4> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(Matrix4 m, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}, {}, {}], [{}, {}, {}, {}], [{}, {}, {}, {}], [{}, {}, {}, {}]", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
	}
};