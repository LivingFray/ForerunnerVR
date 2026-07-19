#pragma once
#include "payload/delta/blam/animations/animation_channel.h"
#include "payload/delta/blam/memory/data.h"
#include <cstdint>

struct c_animation_manager
{
	struct c_animation_channel channels[3];
	uint8_t field1_0x6c;
	uint8_t field2_0x6d;
	uint8_t field3_0x6e;
	uint8_t field4_0x6f;
	uint8_t field5_0x70;
	uint8_t field6_0x71;
	uint8_t field7_0x72;
	uint8_t field8_0x73;
	struct datum jmad_tag_datum;
	uint8_t field10_0x78;
	uint8_t field11_0x79;
	uint8_t field12_0x7a;
	uint8_t field13_0x7b;
	uint8_t field14_0x7c;
	uint8_t field15_0x7d;
	uint8_t field16_0x7e;
	uint8_t field17_0x7f;
	uint8_t field18_0x80;
	uint8_t field19_0x81;
	uint8_t field20_0x82;
	uint8_t field21_0x83;
	uint8_t field22_0x84;
	uint8_t field23_0x85;
	uint8_t field24_0x86;
	uint8_t field25_0x87;
	uint8_t field26_0x88;
	uint8_t field27_0x89;
	uint8_t field28_0x8a;
	uint8_t field29_0x8b;
	float field30_0x8c;
	float field31_0x90;
	float field32_0x94;
	float field33_0x98;
	uint8_t field34_0x9c;
	uint8_t field35_0x9d;
	uint8_t field36_0x9e;
	uint8_t field37_0x9f;
	uint8_t field38_0xa0;
	uint8_t field39_0xa1;
	uint8_t field40_0xa2;
	uint8_t field41_0xa3;
	uint8_t field42_0xa4;
	uint8_t field43_0xa5;
	uint8_t field44_0xa6;
	uint8_t field45_0xa7;
	uint8_t field46_0xa8;
	uint8_t field47_0xa9;
	uint8_t field48_0xaa;
	uint8_t field49_0xab;
	uint8_t field50_0xac;
	uint8_t field51_0xad;
	uint8_t field52_0xae;
	uint8_t field52_0xaf;
};
static_assert(sizeof(c_animation_manager) == 0xb0);