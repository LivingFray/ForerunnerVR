#pragma once
#include <cstdint>
#include "common/utils/log.h"
#include "payload/delta/blam/math/real_math.h"
#include "common/vr/IVR.h"

FORERUNNER_CREATE_LOG_CATEGORY(Delta_Input)

class InputComponent
{
public:

	void RegisterInputs();

	void UpdateInputs(struct simulation_update* update);

protected:

	real_vector2d CalculateMovementInput();

	void ModifyInputVector(struct Vector2& Input, EVR_Controller Controller) const;

	InputBindingID BindingMove;
};