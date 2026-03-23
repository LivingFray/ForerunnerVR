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

		static inline Vector3 Identity()
		{
			return {0.0f, 0.0f, 0.0f};
		}

		static inline float Dot(const Vector3& a, const Vector3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		static inline Vector3 Cross(const Vector3& a, const Vector3& b)
		{
			return {
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
			};
		}
	};

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		static inline Vector4 Identity()
		{
			return {0.0f, 0.0f, 0.0f, 0.0f};
		}

		static inline float Dot(const Vector4& a, const Vector4& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		static inline Vector4 Cross(const Vector4& a, const Vector4& b)
		{
			return {
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x,
				0.0f
			};
		}
	};

	struct Matrix3x3
	{
		float m[3][3];

		static inline Matrix3x3 Identity()
		{
			Matrix3x3 result = {};
			result.m[0][0] = 1.0f;
			result.m[1][1] = 1.0f;
			result.m[2][2] = 1.0f;
			return result;
		}
	};

	struct Matrix4x4
	{
		float m[4][4];

		Vector3 GetTranslation()
		{
			return {m[3][0], m[3][1], m[3][2]};
		}

		Vector4 GetTranslation4()
		{
			return {m[3][0], m[3][1], m[3][2], m[3][3]};
		}

		static inline Matrix4x4 Identity()
		{
			Matrix4x4 result = {};
			result.m[0][0] = 1.0f;
			result.m[1][1] = 1.0f;
			result.m[2][2] = 1.0f;
			result.m[3][3] = 1.0f;
			return result;
		}

		static inline Matrix4x4 FromRotationTranslation(const Matrix3x3& rotation, const Vector3& translation)
		{
			Matrix4x4 result = Identity();
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					result.m[i][j] = rotation.m[i][j];
				}
			}
			result.m[3][0] = translation.x;
			result.m[3][1] = translation.y;
			result.m[3][2] = translation.z;
			return result;
		}

		static inline Matrix3x3 ExtractRotation(const Matrix4x4& matrix)
		{
			Matrix3x3 result = {};
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					result.m[i][j] = matrix.m[i][j];
				}
			}
			return result;
		}

		static inline Matrix4x4 FromBasisVectors(const Vector3& x, const Vector3& y, const Vector3& z)
		{
			Matrix4x4 result = Identity();
			result.m[0][0] = x.x;
			result.m[0][1] = x.y;
			result.m[0][2] = x.z;
			result.m[1][0] = y.x;
			result.m[1][1] = y.y;
			result.m[1][2] = y.z;
			result.m[2][0] = z.x;
			result.m[2][1] = z.y;
			result.m[2][2] = z.z;
			return result;
		}

		static inline Matrix4x4 Translation(float x, float y, float z)
		{
			Matrix4x4 result = Identity();
			result.m[3][0] = x;
			result.m[3][1] = y;
			result.m[3][2] = z;
			return result;
		}

		static inline Matrix4x4 Translation(const Vector3& translation)
		{
			return Translation(translation.x, translation.y, translation.z);
		}

		static inline Matrix4x4 Scale(const Vector3& scale)
		{
			Matrix4x4 result = {};
			result.m[0][0] = scale.x;
			result.m[1][1] = scale.y;
			result.m[2][2] = scale.z;
			result.m[3][3] = 1.0f;
			return result;
		}

		static inline Matrix4x4 RotationX(float angleRadians)
		{
			Matrix4x4 result = Identity();
			float c = cosf(angleRadians);
			float s = sinf(angleRadians);
			result.m[1][1] = c;
			result.m[1][2] = s;
			result.m[2][1] = -s;
			result.m[2][2] = c;
			return result;
		}

		static inline Matrix4x4 RotationY(float angleRadians)
		{
			Matrix4x4 result = Identity();
			float c = cosf(angleRadians);
			float s = sinf(angleRadians);
			result.m[0][0] = c;
			result.m[0][2] = -s;
			result.m[2][0] = s;
			result.m[2][2] = c;
			return result;
		}

		static inline Matrix4x4 RotationZ(float angleRadians)
		{
			Matrix4x4 result = Identity();
			float c = cosf(angleRadians);
			float s = sinf(angleRadians);
			result.m[0][0] = c;
			result.m[0][1] = s;
			result.m[1][0] = -s;
			result.m[1][1] = c;
			return result;
		}

		static inline Matrix4x4 RotationAxis(const Vector3& axis, float angleRadians)
		{
			Matrix4x4 result = Identity();
			float c = cosf(angleRadians);
			float s = sinf(angleRadians);
			float t = 1.0f - c;
			result.m[0][0] = c + axis.x * axis.x * t;
			result.m[0][1] = axis.x * axis.y * t + axis.z * s;
			result.m[0][2] = axis.x * axis.z * t - axis.y * s;
			result.m[1][0] = axis.y * axis.x * t - axis.z * s;
			result.m[1][1] = c + axis.y * axis.y * t;
			result.m[1][2] = axis.y * axis.z * t + axis.x * s;
			result.m[2][0] = axis.z * axis.x * t + axis.y * s;
			result.m[2][1] = axis.z * axis.y * t - axis.x * s;
			result.m[2][2] = c + axis.z * axis.z * t;
			return result;
		}

		inline Matrix4x4 InvertAffine()
		{
			Matrix4x4 result = Identity();
			// Transpose the upper-left 3x3 rotation part
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					result.m[i][j] = m[j][i];
				}
			}
			// Invert the translation
			result.m[3][0] = -(result.m[0][0] * m[3][0] + result.m[1][0] * m[3][1] + result.m[2][0] * m[3][2]);
			result.m[3][1] = -(result.m[0][1] * m[3][0] + result.m[1][1] * m[3][1] + result.m[2][1] * m[3][2]);
			result.m[3][2] = -(result.m[0][2] * m[3][0] + result.m[1][2] * m[3][1] + result.m[2][2] * m[3][2]);
			return result;
		}
	};

	// Vector Operators
	inline Vector3 operator+(const Vector3& a, const Vector3& b)
	{
		return { a.x + b.x, a.y + b.y, a.z + b.z };
	}

	inline Vector3 operator-(const Vector3& a, const Vector3& b)
	{
		return { a.x - b.x, a.y - b.y, a.z - b.z };
	}

	inline Vector3 operator*(const Vector3& v, float scalar)
	{
		return { v.x * scalar, v.y * scalar, v.z * scalar };
	}

	inline Vector3 operator*(float scalar, const Vector3& v)
	{
		return v * scalar;
	}

	inline Vector3 operator/(const Vector3& v, float scalar)
	{
		return { v.x / scalar, v.y / scalar, v.z / scalar };
	}

	inline Vector4 operator+(const Vector4& a, const Vector4& b)
	{
		return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
	}

	inline Vector4 operator-(const Vector4& a, const Vector4& b)
	{
		return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
	}

	inline Vector4 operator*(const Vector4& v, float scalar)
	{
		return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
	}

	inline Vector4 operator*(float scalar, const Vector4& v)
	{
		return v * scalar;
	}

	inline Vector4 operator/(const Vector4& v, float scalar)
	{
		return { v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar };
	}

	// Matrix operations
	inline Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b)
	{
		Matrix4x4 result = {};
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				result.m[row][col] =
					a.m[row][0] * b.m[0][col] +
					a.m[row][1] * b.m[1][col] +
					a.m[row][2] * b.m[2][col] +
					a.m[row][3] * b.m[3][col];
			}
		}
		return result;
	}

	inline Vector4 operator*(const Matrix4x4& m, const Vector4& v)
	{
		return {
			m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
			m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
			m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
			m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w
		};
	}
};