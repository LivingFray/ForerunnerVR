#pragma once
#include <cstdint>

struct datum
{
	int16_t id;
	int16_t salt;
};
static_assert(sizeof(datum) == 0x4);



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