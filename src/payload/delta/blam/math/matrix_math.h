#pragma once
#include "payload/forerunner/patch.h"

FUNCTION("halo2.dll", 0x7299f0, "?? 89 5c ?? 08 57 ?? 83 ec 20 ?? 8b da ?? 8b f9 e8 ?? ?? ?? ?? f2 0f 10 43 10 f2 0f 11 47 28", 0, void, matrix4x3_from_orientation, struct real_matrix4x3* matrix, struct real_orientation* orientation);
FUNCTION("halo2.dll", 0x72a150, "48 83 ec 48 ?? 3b c8 75 ?? 0f 10 01 8b 41 30 0f 10 49 10 89 44 ?? 30 0f 11 04 ?? 0f 10 41 20 ?? 8d 0c ??", 0, void, matrix4x3_multiply, struct real_matrix4x3* a, struct real_matrix4x3* b, struct real_matrix4x3* result);