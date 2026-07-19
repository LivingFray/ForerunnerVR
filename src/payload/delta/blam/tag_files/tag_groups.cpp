#include "tag_groups.h"

void* tag_block_get_element(tag_block tag)
{
	if (tag.address == -1)
	{
		return nullptr;
	}

	if (tag.address < 0)
	{
		return (void*)((int64_t)g_tags_physical_memory() - tag.address);
	}
	else
	{
		return (void*)((int64_t)g_cache_file_globals__tags_header() + tag.address);
	}
}