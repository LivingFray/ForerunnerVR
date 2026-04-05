#include "data.h"
#include "common/utils/log.h"

FORERUNNER_CREATE_LOG_CATEGORY(Data);

void* data_get_datum(data_array* data, datum index)
{
	if (!data)
	{
		FORERUNNER_ERROR(Data, "Attempted to get datum from null data_array");
		return nullptr;
	}

	if (index.id < 0 || index.id >= static_cast<int32_t>(data->count))
	{
		FORERUNNER_ERROR(Data, "Attempted to get datum from '{}' with index {} (count = {})", data->name, index, data->count);
		return nullptr;
	}

	int32_t offset = index.id * data->size;
	unsigned char* data_start = reinterpret_cast<unsigned char*>(data) + data->data;

	return &(data_start[offset]);
}

void* data_get_pooled(data_pool* pool, int32_t offset)
{
	if (!pool)
	{
		FORERUNNER_ERROR(Data, "Attempted to get pooled object from null data_pool");
		return nullptr;
	}

	// No idea why the pointer arithmetic is so weird here, I just copied what ghidra spat out
	return reinterpret_cast<void*>(((reinterpret_cast<int64_t>(pool) + 0x57) & -0x10) + offset);
}
