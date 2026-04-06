#include "first_person_weapons.h"
#include "payload/delta/deltamodule.h"
#include "common/utils/utils.h"
#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/objects/objects.h"

int first_person_update_bones::Patch(int user_index, datum object_id, real_vector3d* translation, real_vector3d* facing, real_vector3d* up, int maximum_model_count, s_model* models, bool param_8)
{
	real_vector3d CameraLocation = SameCast<real_vector3d>(DeltaModule::Get().Camera.GetGameCameraLocation());

	object_datum* player = player_get_object_datum(0);

	if (!player)
	{
		return Original(user_index, object_id, &CameraLocation, facing, up, maximum_model_count, models, param_8);
	}

	// NB: This doesn't account for interpolation, turning leads to stuttery movement
	return Original(user_index, object_id, &CameraLocation, &player->object.forward, &player->object.up, maximum_model_count, models, param_8);
}