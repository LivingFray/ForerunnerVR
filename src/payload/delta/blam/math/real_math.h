#pragma once

struct real_vector2d
{
	float x;
	float y;
};
static_assert(sizeof(real_vector2d) == 0x8);

struct real_point2d
{
	float x;
	float y;
};
static_assert(sizeof(real_point2d) == 0x8);

struct real_vector3d
{
	float x;
	float y;
	float z;
};
static_assert(sizeof(real_vector3d) == 0xc);

struct real_point3d
{
	float x;
	float y;
	float z;
};
static_assert(sizeof(real_point3d) == 0xc);

struct real_vector4d
{
	float x;
	float y;
	float z;
	float w;
};
static_assert(sizeof(real_vector4d) == 0x10);

struct real_point4d
{
	float x;
	float y;
	float z;
	float w;
};
static_assert(sizeof(real_point4d) == 0x10);

struct real_plane3d
{
	real_vector3d n;
	float d;
};
static_assert(sizeof(real_plane3d) == 0x10);

struct real_matrix3x3
{
	float matrix[3][3];
};
static_assert(sizeof(real_matrix3x3) == 0x24);

struct real_matrix4x3
{
	float scale;
	real_matrix3x3 rotation;
	real_vector3d translation;
};
static_assert(sizeof(real_matrix4x3) == 0x34);

struct real_matrix4x4
{
	float matrix[4][4];
};
static_assert(sizeof(real_matrix4x4) == 0x40);

struct real_rgb_color
{
	float r;
	float g;
	float b;
};
static_assert(sizeof(real_rgb_color) == 0xc);

struct real_argb_color
{
	float a;
	float r;
	float g;
	float b;
};
static_assert(sizeof(real_argb_color) == 0x10);

struct real_rectangle2d
{
	float x0;
	float x1;
	float y0;
	float y1;
};
static_assert(sizeof(real_rectangle2d) == 0x10);

struct euler_angles2d
{
	float yaw;
	float pitch;
};
static_assert(sizeof(euler_angles2d) == 0x8);

struct real_quaternion
{
	real_vector3d v;
	float w;
};
static_assert(sizeof(real_quaternion) == 0x10);

struct real_orientation
{
	real_quaternion rotation;
	real_vector3d translation;
	float scale;
};
static_assert(sizeof(real_orientation) == 0x20);


// Formatting
#include <format>
template <>
struct std::formatter<real_vector2d> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(real_vector2d v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}]", v.x, v.y);
	}
};
template <>
struct std::formatter<real_vector3d> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(real_vector3d v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}, {}]", v.x, v.y, v.z);
	}
};
template <>
struct std::formatter<real_vector4d> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(real_vector4d v, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {}, {}, {}]", v.x, v.y, v.z, v.w);
	}
};

template <>
struct std::formatter<euler_angles2d> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(euler_angles2d e, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[yaw = {}, pitch = {}]", e.yaw, e.pitch);
	}
}; 
template <>
struct std::formatter<real_matrix3x3> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(real_matrix3x3 m, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]]", m.matrix[0][0], m.matrix[0][1], m.matrix[0][2], m.matrix[1][0], m.matrix[1][1], m.matrix[1][2], m.matrix[2][0], m.matrix[2][1], m.matrix[2][2]);
	}
};
template <>
struct std::formatter<real_matrix4x3> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(real_matrix4x3 m, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[scale = {}, rotation = {}, translation = {}]", m.scale, m.rotation, m.translation);
	}
};