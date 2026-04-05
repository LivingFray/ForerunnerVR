#pragma once
#include <cstdint>

struct datum
{
	int16_t id = -1;
	int16_t salt = -1;
};
static_assert(sizeof(datum) == 0x4);

struct data_array
{
	char name[32];
	int32_t maximum_count;
	int32_t size;
	bool valid;
	uint8_t unk0;
	uint16_t flags;
	char tag[4];
	void* allocator;
	uint32_t unk1;
	uint32_t count;
	uint8_t unk3[8];
	uint64_t data;
	uint64_t extra_data;
};
static_assert(sizeof(data_array) == 0x58);

struct data_pool
{
	char tag[4];
	char name[32];
	uint8_t unk0[4];
	void* allocator;
	int32_t size;
	int32_t capacity;
	uint8_t unk1[32];
};
static_assert(sizeof(data_pool) == 0x58);


// Helper functions based on similar functions found in blam, but not necessarily 1:1 decomps
void* data_get_datum(data_array* data, datum index);
void* data_get_pooled(data_pool* pool, int32_t offset);


// Formatting
#include <format>
template <>
struct std::formatter<datum> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(datum d, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "[id: {}, salt: {}]", d.id, d.salt);
	}
};