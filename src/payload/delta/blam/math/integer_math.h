#pragma once

struct rectangle2d
{
	short y0;
	short x0;
	short y1;
	short x1;
};
static_assert(sizeof(rectangle2d) == 0x8);