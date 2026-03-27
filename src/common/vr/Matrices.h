#pragma once
#include <type_traits>
#include <math.h>

// Module-independent vector + matrix types for VR interfaces to use

namespace VR
{
	// Assuming underlying layout is the same types can be converted between module specific types with this helper
	template <typename To, typename From>
	const To& Cast(const From& from)
	{
		static_assert(std::is_standard_layout_v<From>, "From type must be standard layout");
		static_assert(std::is_standard_layout_v<To>, "To type must be standard layout");
		static_assert(sizeof(To) == sizeof(From), "Types must be the same size");

		return reinterpret_cast<const To&>(from);
	}

	struct Vector3
	{
		float x;
		float y;
		float z;

		static Vector3 Identity();
		static Vector3 FromVector4(const struct Vector4& v);
		static float Dot(const Vector3& a, const Vector3& b);
		static Vector3 Cross(const Vector3& a, const Vector3& b);

		float Length() const;
		float LengthSqr() const;

		void Normalize();
		bool SafeNormalize();
	};

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		static Vector4 Identity();
		static Vector4 FromPoint(const Vector3& v);
		static Vector4 FromVector(const Vector3& v);
		static float Dot(const Vector4& a, const Vector4& b);
		static Vector4 Cross(
			const Vector4& a,
			const Vector4& b);
	};

	struct Matrix3x3
	{
		float m[3][3];

		static Matrix3x3 Identity();
	};

	struct Matrix4x4
	{
		float m[4][4];

		Vector3 GetTranslation();
		Vector4 GetTranslation4();
		static Matrix4x4 Identity();
		static Matrix4x4 FromRotationTranslation(const Matrix3x3& rotation, const Vector3& translation);
		static Matrix3x3 ExtractRotation(const Matrix4x4& matrix);
		static Matrix4x4 FromBasisVectors(const Vector3& x, const Vector3& y, const Vector3& z);
		static Matrix4x4 Translation(float x, float y, float z);
		static Matrix4x4 Translation(const Vector3& translation);
		static Matrix4x4 Scale(float x, float y, float z);
		static Matrix4x4 Scale(float s);
		static Matrix4x4 Scale(const Vector3& scale);
		static Matrix4x4 RotationX(float angleRadians);
		static Matrix4x4 RotationY(float angleRadians);
		static Matrix4x4 RotationZ(float angleRadians);
		static Matrix4x4 RotationAxis(const Vector3& axis, float angleRadians);
		Matrix4x4 InvertAffine();
	};

	// Vector Operators
	Vector3 operator+(const Vector3& a, const Vector3& b);
	Vector3 operator-(const Vector3& a, const Vector3& b);
	Vector3 operator*(const Vector3& v, float scalar);
	Vector3 operator*(float scalar, const Vector3& v);
	Vector3 operator/(const Vector3& v, float scalar);

	Vector4 operator+(const Vector4& a, const Vector4& b);
	Vector4 operator-(const Vector4& a, const Vector4& b);
	Vector4 operator*(const Vector4& v, float scalar);
	Vector4 operator*(float scalar, const Vector4& v);
	Vector4 operator/(const Vector4& v, float scalar);

	// Matrix operations
	Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b);
	Vector4 operator*(const Matrix4x4& m, const Vector4& v);
}

// Formatting
#include <format>
template <>
struct std::formatter<VR::Vector3> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(VR::Vector3 v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}, {}]", v.x, v.y, v.z);
	}
};
template <>
struct std::formatter<VR::Vector4> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(VR::Vector4 v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}, {}, {}]", v.x, v.y, v.z, v.w);
	}
};

template <>
struct std::formatter<VR::Matrix3x3> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(VR::Matrix3x3 m, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[\n{}, {}, {},\n{}, {}, {},\n{}, {}, {}\n]",
			m.m[0][0], m.m[1][0], m.m[2][0],
			m.m[0][1], m.m[1][1], m.m[2][1],
			m.m[0][2], m.m[1][2], m.m[2][2]);
	}
};

template <>
struct std::formatter<VR::Matrix4x4> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(VR::Matrix4x4 m, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[\n{}, {}, {}, {},\n{}, {}, {}, {},\n{}, {}, {}, {},\n{}, {}, {}, {}\n]",
			m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
			m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
			m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
			m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
	}
};