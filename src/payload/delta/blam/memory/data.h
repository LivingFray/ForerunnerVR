#pragma once

struct datum
{
	short id;
	short salt;
};
static_assert(sizeof(datum) == 0x4);