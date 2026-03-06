#include "rasterizer_present.h"
#include "payload/delta/deltamodule.h"

void rasterizer_present::Patch()
{
	DeltaModule::Get().Present();
	Original();
}