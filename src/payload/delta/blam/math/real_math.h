#pragma once

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

struct real_plane3d
{
	real_vector3d n;
	float d;
};
static_assert(sizeof(real_plane3d) == 0x10);