#pragma once
#include <cstdint>
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/objects/object_location.h"

// todo: finish this
struct observer
{
	uint32_t header_signature;

	uint32_t trailer_signature;
};

struct camera_result
{
	real_vector3d position;
	object_location location;
	real_vector3d velocity;
	real_vector3d forward;
	real_vector3d up;
	float hozitonal_field_of_view;
	float vertical_field_of_view;
	//...
};