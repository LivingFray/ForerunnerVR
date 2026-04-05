#pragma once
#include "payload/forerunner/patch.h"

#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/memory/data.h"

struct s_model
{
	// todo: fill in
};

PATCH("halo2.dll", 0x8181f0, "?? 89 5c ?? 18 ?? 89 4c ?? 20 89 54 ?? 10 89 4c ?? 08 55 56 57 ?? 54 ?? 55 ?? 56 ?? 57 ?? 8d ac ?? 50 ff ff ff ?? 81 ec b0 01 00 00 ?? 33 e4", first_person_update_bones, int, int user_index, datum object_id, real_vector3d* translation, real_vector3d* facing, real_vector3d* up, int maximum_model_count, s_model* models, bool param_8);