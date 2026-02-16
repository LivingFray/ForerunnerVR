#pragma once
#include <cstdint>

struct object_location
{
	int32_t leaf_index;
	int16_t cluster_index;
	int16_t bsp_index;
};
static_assert(sizeof(object_location) == 0x8);