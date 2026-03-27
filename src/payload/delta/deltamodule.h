#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include "common/utils/singleton.h"
#include "common/utils/log.h"
#include "payload/forerunner/patch.h"
// Components
#include "rendercomponent.h"
#include "uicomponent.h"
#include "cameracomponent.h"

FORERUNNER_CREATE_LOG_CATEGORY(Delta);

class DeltaModule : public Singleton<DeltaModule>
{
public:
	bool Initialise();
	void Deinitialise();

	void PreUpdate(int ticks, float* seconds);
	void PostUpdate(int ticks, float* seconds);

	void Present();

	class IVR* VR = nullptr;

	RenderComponent Render;
	UIComponent UI;
	CameraComponent Camera;

protected:
	static inline const char* ModuleName = "halo2.dll";
	HMODULE GameModule;

	bool CreatePatches();
	bool ApplyPatches();
	bool FindGlobals();
	bool PatchCode();
	bool DisablePatches();
	bool DestroyPatches();

	bool PatchSplitscreen();

public:

	bool bRenderingHUD = false;
};

OFFSET(update_player_views__valid_user_id, "halo2.dll", 0x9602a8, "ff c3 83 fb 04 7d ?? 8b cb e8 ?? ?? ?? ?? 84 c0 75 ?? ff c3 83 fb 04", +7);
OFFSET(update_player_views__get_camera_result, "halo2.dll", 0x9602fc, "be 01 00 00 00 8d 6e 01 eb ?? 83 fb 04 7d ?? 83 fb ff 74 ?? e8 ?? ?? ?? ?? 84 c0 74 ?? 8b cb e8 ?? ?? ?? ?? 48 85 c0 74 ?? 48 89 44 24 30 44 8b cf 89 5c 24 28 44 8b c6 33 d2 44 89 7c 24 20", +29);
OFFSET(calculate_viewport__left, "halo2.dll", 0x7e0b35, "66 83 42 02 04 41 0f bf 06 0f bf 0e 03 c8 0f bf 45 00", +4);
OFFSET(calculate_viewport__right, "halo2.dll", 0x7e0b56, "66 83 42 06 fc 66 83 7e 02 00 75 ?? 0f b7 44 24 50 66 89 07", +4);
OFFSET(calculate_viewport__bottom, "halo2.dll", 0x7e0b6b, "66 83 02 04 41 0f bf 46 02 0f bf 4e 02 03 c8 0f bf 45 02 3b c8", +3);
OFFSET(calculate_viewport__top, "halo2.dll", 0x7e0b9a, "66 83 42 04 fc 48 8b 6c 24 58 48 83 c4 20 41 5e 5f 5e c3", +4);