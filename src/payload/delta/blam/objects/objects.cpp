#include "objects.h"

object_header_datum* objects_get_header_datum(datum index)
{
	return reinterpret_cast<object_header_datum*>(data_get_datum(g_objects(), index));
}

object_datum* objects_get_datum(datum index)
{
	object_header_datum* Header = objects_get_header_datum(index);
	if (!Header)
	{
		return nullptr;
	}

	return reinterpret_cast<object_datum*>(data_get_pooled(g_objects_pool(), Header->pool_offset));
}
