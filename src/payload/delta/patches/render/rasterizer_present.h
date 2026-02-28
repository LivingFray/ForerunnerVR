#pragma once

#include "payload/forerunner/patch.h"

PATCH("halo2.dll", 0x9523c0, "", rasterizer_present, void);