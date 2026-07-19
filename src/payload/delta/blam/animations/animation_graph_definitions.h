#pragma once
#include "payload/forerunner/patch.h"
#include "common/utils/log.h"

#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/tag_files/tag_groups.h"
#include <cstdint>

FORERUNNER_CREATE_LOG_CATEGORY(Delta_AnimGraph);

struct c_animation2_rig_definition
{
	void* vtbl;
	uint8_t unk0[4];
	tag_block nodes;
};
static_assert(sizeof(c_animation2_rig_definition) == 0x18);

struct c_animation2_rig_node
{
	int32_t name;
	int16_t next_sibling_node_index;
	int16_t first_child_node_index;
	int16_t parent_node_index;
	int16_t pad;
	int32_t flags;
	real_vector3d base_vector;
	float range;
};
static_assert(sizeof(c_animation2_rig_node) == 0x20);

PATCH("halo2.dll", 0x79f810, "", c_animation2_rig_definition__node_matrices_from_orientations, void, c_animation2_rig_definition*, struct real_matrix4x3*, struct real_orientation*, struct real_matrix4x3*);
PATCH("halo2.dll", 0x79f940, "", c_animation2_rig_definition__node_matrices_from_orientations_with_gun_hand_swap, void, c_animation2_rig_definition*, struct real_matrix4x3*, struct real_orientation*, struct real_matrix4x3*, short child_node_index, short alternate_parent_node_matrix);