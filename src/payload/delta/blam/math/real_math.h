#pragma once

struct real_vector2d
{
	float x;
	float y;
};
static_assert(sizeof(real_vector2d) == 0x8);

using real_point2d = real_vector2d;

struct real_vector3d
{
	float x;
	float y;
	float z;
};
static_assert(sizeof(real_vector3d) == 0xc);

using real_point3d = real_vector3d;

struct real_vector4d
{
	float x;
	float y;
	float z;
	float w;
};
static_assert(sizeof(real_vector4d) == 0x10);

using real_point4d = real_vector4d;

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