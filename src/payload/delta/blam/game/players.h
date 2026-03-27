#pragma once
#include <cstdint>
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/memory/data.h"
#include "payload/delta/blam/game/aim_assist.h"
#include "payload/forerunner/patch.h"

struct players_globals
{
	int32_t player_count;
	bool unk0;
	bool unk1;
	bool input_disabled;
	bool movement_disabled;
	int16_t player_user_count;
	int16_t player_controller_count;
	datum player_ids[4];
	datum player_objects[4];
	uint8_t unk2[304];
};
static_assert(sizeof(players_globals) == 0x15c);

GLOBAL(players_globals*, g_players_globals, "halo2.dll", 0xe80a20, "41 b8 40 04 00 00 48 8d 0d ?? ?? ?? ?? 48 8b d8 e8 ?? ?? ?? ?? 33 c9 66 89 4b 08", +30);

#define BIT(x) (1u << x)

//TODO: Find where this gets set in code and find the actual purposes
enum class e_control_flags : uint32_t
{
	_none = 0,
	_crouch = BIT(0),
	_jump = BIT(1),
	_vehicle3 = BIT(2),
	_vehicle2 = BIT(3),
	_flashlight = BIT(4),
	_bit5 = BIT(5),
	_bit6 = BIT(6), // Gets set with melee, but not needed
	_melee = BIT(7),
	_bit8 = BIT(8), // Gets set when firing, doesn't trigger firing though (primary)
	_previous_grenade = BIT(9),
	_next_grenade = BIT(10),
	_bit11 = BIT(11),
	_bit12 = BIT(12),
	_bit13 = BIT(13), // present on dual wield right fire
	_bit14 = BIT(14),
	_bit15 = BIT(15),
	_bit16 = BIT(16), // present on dual wield right fire, present on push to talk with controller
	_bit17 = BIT(17),
	_fire_right = BIT(18), // present on dual wield right fire
	_bit19 = BIT(19), // present on dual wield left fire, present on 1 weapon fire
	_fire_right_2 = BIT(20),
	_bit21 = BIT(21),
	_bit22 = BIT(22),
	_bit23 = BIT(23), // present on dual wield left fire
	_bit24 = BIT(24),
	_bit25 = BIT(25),
	_bit26 = BIT(26),
	_bit27 = BIT(27),
	_throw_grenade = BIT(28),
	_bit29 = BIT(29),
	_bit30 = BIT(30),
	_banshee_bomb = BIT(31),
	// Observed compound inputs
	_vehicle1 = BIT(13) | BIT(16),
	_fire = BIT(8) | BIT(18) | BIT(19) | BIT(23),
	_left_weapon = BIT(23) | BIT(19) | BIT(16) | BIT(13),
};

#undef BIT

struct player_action
{
	uint32_t control_flags;
	uint32_t field1_0x4;
	euler_angles2d rotation;
	real_vector2d throttle;
	float primary_trigger;
	float secondary_trigger;
	uint32_t flags;
	uint8_t field7_0x24;
	uint8_t field8_0x25;
	int8_t primary_weapon_index;
	int8_t secondary_weapon_index;
	int16_t grenade_index;
	int16_t zoom_level;
	int16_t field13_0x2c;
	uint8_t field14_0x2e;
	uint8_t field15_0x2f;
	int32_t field16_0x30;
	uint8_t field17_0x34;
	uint8_t field18_0x35;
	uint8_t field19_0x36;
	uint8_t field20_0x37;
	aim_assist_targeting_result aim_assist;
	uint8_t field22_0x5c;
	uint8_t field23_0x5d;
	uint8_t field24_0x5e;
	uint8_t field25_0x5f;
};
static_assert(sizeof(player_action) == 0x60);

PATCH("halo2.dll", 0x6a4380, "48 8b 05 ?? ?? ?? ?? 0f bf 40 08 c3", players_get_window_count, int);

PATCH("halo2.dll", 0x6a3910, "48 89 5c 24 08 48 89 74 24 18 48 89 7c 24 20 55 41 54 41 55 41 56 41 57 48 8d ac 24 30 ff ff ff 48 81 ec d0 01 00 00 4c 8b e9", players_update_before_game, void, struct simulation_update* update);