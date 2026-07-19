#pragma once

#include <cstdint>

struct c_interpolator_control
{
	byte m_ticks_remaining;
	byte m_duration_ticks;
	byte m_interpolation_type;
	byte m_flags;
};
