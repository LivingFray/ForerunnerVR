#pragma once
#include "payload/forerunner/patch.h"
#include <cstdint>

struct tag_block
{
	int32_t count;
	int32_t address;
};
static_assert(sizeof(tag_block) == 0x8);

GLOBAL(void*, g_cache_file_globals__tags_header, "halo2.dll", 0xe80ab0, "?? 63 54 b4 60 33 c0 ?? 63 4d 10 ?? ff c6 83 f9 ff 74 ?? 85 c9 79 ?? 8b c1 0f ba f0 1f ?? 03 05", +44);
GLOBAL(void*, g_tags_physical_memory, "halo2.dll", 0xe80ac0, "?? 63 54 b4 60 33 c0 ?? 63 4d 10 ?? ff c6 83 f9 ff 74 ?? 85 c9 79 ?? 8b c1 0f ba f0 1f ?? 03 05", +32);

// Function appears to get inlined in MCC, as its code structure is seen in many places
// Name and file is therefore unconfirmed
void* tag_block_get_element(tag_block tag);