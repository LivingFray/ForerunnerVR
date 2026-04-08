#include "openvr.h"
#include "common/utils/matrices.h"
#include "common/utils/utils.h"
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
{
	Matrix4 matrixObj(
		matPose.m[2][2], matPose.m[0][2], -matPose.m[1][2], 0.0,
		matPose.m[2][0], matPose.m[0][0], -matPose.m[1][0], 0.0,
		-matPose.m[2][1], -matPose.m[0][1], matPose.m[1][1], 0.0,
		-matPose.m[2][3], -matPose.m[0][3], matPose.m[1][3], 1.0f
	);

	return matrixObj;
}

bool OpenVR::EarlyInit()
{
	FORERUNNER_LOG(OpenVR, "Initialising OpenVR");

	vr::EVRInitError initError = vr::VRInitError_None;
	VRSystem = vr::VR_Init(&initError, vr::VRApplication_Scene);

	if (initError != vr::VRInitError_None)
	{
		FORERUNNER_ERROR(OpenVR, "VR_Init failed : {}", vr::VR_GetVRInitErrorAsEnglishDescription(initError));
		return false;
	}

	VRCompositor = vr::VRCompositor();

	if (!VRCompositor)
	{
		FORERUNNER_ERROR(OpenVR, "VRCompositor failed");
		return false;
	}

	VROverlay = vr::VROverlay();

	if (!VROverlay)
	{
		FORERUNNER_ERROR(OpenVR, "VROverlay failed");
		return false;
	}

	VRInput = vr::VRInput();

	if (!VRInput)
	{
		FORERUNNER_ERROR(OpenVR, "VRInput failed");
		return false;
	}

	std::filesystem::path Manifest = ForerunnerPath / "VR" / "OpenVR" / "forerunner.vrmanifest";
	vr::EVRApplicationError AppErr = vr::VRApplications()->AddApplicationManifest(Manifest.string().c_str());

	if (AppErr != vr::VRApplicationError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not add application manifest: {}", static_cast<int32_t>(AppErr));
	}

	AppErr = vr::VRApplications()->IdentifyApplication(0, "livingfray.forerunner");

	if (AppErr != vr::VRApplicationError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not set id: {}", static_cast<int32_t>(AppErr));
	}

	std::filesystem::path Actions = ForerunnerPath / "VR" / "OpenVR" / "actions.json";
	vr::EVRInputError InputErr = VRInput->SetActionManifestPath(Actions.string().c_str());

	if (InputErr != vr::VRInputError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not set action manifest path: {}", static_cast<int32_t>(InputErr));
	}

	// TODO: Init poses/active action sets here

	VRSystem->GetRecommendedRenderTargetSize(&RecommendedWidth, &RecommendedHeight);

	// Wizardry which converts the asymmetrical projection matrices requested into the cropped symmetrical ones most games expect
	float l_left = 0.0f, l_right = 0.0f, l_top = 0.0f, l_bottom = 0.0f;
	VRSystem->GetProjectionRaw(vr::EVREye::Eye_Left, &l_left, &l_right, &l_top, &l_bottom);
	FORERUNNER_LOG(OpenVR, "Left eye raw projection[l, r, t, b] = [{}, {}, {}, {}]", l_left, l_right, l_top, l_bottom);

	float r_left = 0.0f, r_right = 0.0f, r_top = 0.0f, r_bottom = 0.0f;
	VRSystem->GetProjectionRaw(vr::EVREye::Eye_Right, &r_left, &r_right, &r_top, &r_bottom);
	FORERUNNER_LOG(OpenVR, "Right eye raw projection[l, r, t, b] = [{}, {}, {}, {}]", r_left, r_right, r_top, r_bottom);

	float TanHalfFov[2]{
		(std::max)({-l_left, l_right, -r_left, r_right}),
		(std::max)({-l_top, l_bottom, -r_top, r_bottom})
	};

	FORERUNNER_LOG(OpenVR, "TanHalfFov[horiz, vert] = [{}, {}]", TanHalfFov[0], TanHalfFov[1]);

	TextureBounds[0].uMin = 0.5f + 0.5f * l_left / TanHalfFov[0];
	TextureBounds[0].uMax = 0.5f + 0.5f * l_right / TanHalfFov[0];
	TextureBounds[0].vMin = 0.5f - 0.5f * l_bottom / TanHalfFov[1];
	TextureBounds[0].vMax = 0.5f - 0.5f * l_top / TanHalfFov[1];
	FORERUNNER_LOG(OpenVR, "Left eye TextureBounds[uMin, uMax, vMin, vMax] = [{}, {}, {}, {}]", TextureBounds[0].uMin, TextureBounds[0].uMax, TextureBounds[0].vMin, TextureBounds[0].vMax);

	TextureBounds[1].uMin = 0.5f + 0.5f * r_left / TanHalfFov[0];
	TextureBounds[1].uMax = 0.5f + 0.5f * r_right / TanHalfFov[0];
	TextureBounds[1].vMin = 0.5f - 0.5f * r_bottom / TanHalfFov[1];
	TextureBounds[1].vMax = 0.5f - 0.5f * r_top / TanHalfFov[1];
	FORERUNNER_LOG(OpenVR, "Right eye TextureBounds[uMin, uMax, vMin, vMax] = [{}, {}, {}, {}]", TextureBounds[1].uMin, TextureBounds[1].uMax, TextureBounds[1].vMin, TextureBounds[1].vMax);

	AspectRatio = TanHalfFov[0] / TanHalfFov[1];
	VerticalFieldOfView = 2.0f * atan(TanHalfFov[1]);

	const uint32_t RealWidth = RecommendedWidth;
	const uint32_t RealHeight = RecommendedHeight;

	RecommendedWidth = static_cast<uint32_t>(RecommendedWidth / (std::max)(TextureBounds[0].uMax - TextureBounds[0].uMin, TextureBounds[1].uMax - TextureBounds[1].uMin));
	RecommendedHeight = static_cast<uint32_t>(RecommendedHeight / (std::max)(TextureBounds[0].vMax - TextureBounds[0].vMin, TextureBounds[1].vMax - TextureBounds[1].vMin));

	FORERUNNER_LOG(OpenVR, "Stretched Width/Height from {}x{} to {}x{}", RealWidth, RealHeight, RecommendedWidth, RecommendedHeight);
	FORERUNNER_LOG(OpenVR, "Desired vertical fov = {} Desired aspect ratio = {}", Rad2Deg(VerticalFieldOfView), AspectRatio);

	FORERUNNER_LOG(OpenVR, "VR systems created successfully");

	return true;
}

bool OpenVR::Init()
{
	return true;
}

void OpenVR::Shutdown()
{
	vr::VR_Shutdown();
}

void OpenVR::Update(float DeltaTime)
{
	if (!VRCompositor)
	{
		return;
	}

	VRCompositor->WaitGetPoses(RenderPoses, vr::k_unMaxTrackedDeviceCount, GamePoses, vr::k_unMaxTrackedDeviceCount);

	if (ActiveActionSets.size() > 0)
	{
		vr::EVRInputError InputErr = VRInput->UpdateActionState(ActiveActionSets.data(), sizeof(vr::VRActiveActionSet_t), static_cast<uint32_t>(ActiveActionSets.size()));
		if (InputErr != vr::VRInputError_None)
		{
			FORERUNNER_WARN(OpenVR, "Could not update action state: {}", static_cast<int>(InputErr));
		}
	}
}

void OpenVR::SubmitEye(EVR_Eye Eye, ID3D11Texture2D* Texture, const VR_Bounds& ViewBounds)
{
	// TODO: Query if frame is finished rendering first?

	vr::Texture_t EyeTexture{(void*)Texture, vr::TextureType_DirectX, vr::ColorSpace_Auto};

	vr::VRTextureBounds_t& EyeBounds = TextureBounds[static_cast<int>(Eye)];

	vr::VRTextureBounds_t FinalTextureBounds
	{
		.uMin = ViewBounds.x + EyeBounds.uMin * ViewBounds.w,
		.vMin = ViewBounds.y + EyeBounds.vMin * ViewBounds.h,
		.uMax = ViewBounds.x + EyeBounds.uMax * ViewBounds.w,
		.vMax = ViewBounds.y + EyeBounds.vMax * ViewBounds.h
	};

	vr::EVRCompositorError Error = VRCompositor->Submit(static_cast<vr::EVREye>(Eye), &EyeTexture, &FinalTextureBounds, vr::Submit_Default);

	if (Error != vr::VRCompositorError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not submit {} eye texture: {}", Eye, static_cast<int>(Error));
	}

	VRCompositor->PostPresentHandoff();
}

void OpenVR::SetDevice(ID3D11Device* InDevice)
{
	Device = InDevice;
}

void OpenVR::SetDeviceContext(ID3D11DeviceContext* InContext)
{
	Context = InContext;
}

int32_t OpenVR::GetDesiredWidth() const
{
	return RecommendedWidth;
}

int32_t OpenVR::GetDesiredHeight() const
{
	return RecommendedHeight;
}

float OpenVR::GetVerticalFieldOfView(EVR_Eye Eye) const
{
	return VerticalFieldOfView;
}

Matrix4 OpenVR::GetHMDTransform() const
{
	if (!VRSystem)
	{
		FORERUNNER_WARN(OpenVR, "Attempted to get HMD transform before initialising");
		return Matrix4();
	}

	return ConvertSteamVRMatrixToMatrix4(RenderPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);
}

Matrix4 OpenVR::GetEyeTransform(EVR_Eye Eye) const
{
	if (!VRSystem)
	{
		FORERUNNER_WARN(OpenVR, "Attempted to get {} eye transform before initialising", Eye);
		return Matrix4();
	}

	return ConvertSteamVRMatrixToMatrix4(VRSystem->GetEyeToHeadTransform(static_cast<vr::EVREye>(Eye))).invertAffine();
}

Matrix4 OpenVR::GetControllerTransform(EVR_Controller Controller) const
{
	if (!VRSystem)
	{
		FORERUNNER_WARN(OpenVR, "Attempted to get {} controller transform before initialising", Controller);
		return Matrix4();
	}
	
	vr::TrackedDeviceIndex_t controllerIndex = VRSystem->GetTrackedDeviceIndexForControllerRole(Controller == EVR_Controller::Left ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);

	Matrix4 outMatrix;

	return ConvertSteamVRMatrixToMatrix4(RenderPoses[controllerIndex].mDeviceToAbsoluteTracking);
}

static std::string SetAndActionToString(const std::string& Set, const std::string& Action)
{
	return ("/actions/" + Set + "/in/" + Action);
}

InputBindingID OpenVR::RegisterBoolInput(const std::string& Set, const std::string& Action)
{
	InputBindingID ID = 0;
	std::string InputString = SetAndActionToString(Set, Action);

	vr::EVRInputError InputErr = VRInput->GetActionHandle(InputString.c_str(), &ID);
	if (InputErr != vr::VRInputError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not register bool input {}: {}", InputString, static_cast<int32_t>(InputErr));
	}
	else
	{
		FORERUNNER_LOG(OpenVR, "Registered bool input {} with id {}", InputString, ID);
	}
	return ID;
}

InputBindingID OpenVR::RegisterVector2Input(const std::string& Set, const std::string& Action)
{
	InputBindingID ID = 0;
	std::string InputString = SetAndActionToString(Set, Action);

	vr::EVRInputError InputErr = VRInput->GetActionHandle(InputString.c_str(), &ID);
	if (InputErr != vr::VRInputError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not register Vector2 input {}: {}", InputString, static_cast<int32_t>(InputErr));
	}
	else
	{
		FORERUNNER_LOG(OpenVR, "Registered Vector2 input {} with id {}", InputString, ID);
	}
	return ID;
}

bool OpenVR::GetBoolInput(InputBindingID ID) const
{
	static bool dummy = false;
	return GetBoolInput(ID, dummy);
}

bool OpenVR::GetBoolInput(InputBindingID ID, bool& bHasChanged) const
{
	if (!VRInput)
	{
		FORERUNNER_WARN(OpenVR, "Attempted to get input binding before initialising");
		return false;
	}

	static vr::InputDigitalActionData_t Digital;
	vr::EVRInputError InputErr = VRInput->GetDigitalActionData(ID, &Digital, sizeof(vr::InputDigitalActionData_t), vr::k_ulInvalidInputValueHandle);
	if (InputErr != vr::VRInputError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not get digital action {}: {}", ID, static_cast<int32_t>(InputErr));
		return false;
	}

	bHasChanged = Digital.bChanged;

	return Digital.bState;
}

Vector2 OpenVR::GetVector2Input(InputBindingID ID) const
{
	if (!VRInput)
	{
		FORERUNNER_WARN(OpenVR, "Attempted to get input binding before initialising");
		return Vector2();
	}

	static vr::InputAnalogActionData_t Analog;
	vr::EVRInputError InputErr = VRInput->GetAnalogActionData(ID, &Analog, sizeof(vr::InputAnalogActionData_t), vr::k_ulInvalidInputValueHandle);
	if (InputErr != vr::VRInputError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not get analog action {}: {}", ID, static_cast<int32_t>(InputErr));
		return Vector2();
	}

	return Vector2(Analog.x, Analog.y);
}

InputBindingID OpenVR::RegisterActionSet(const std::string& Set)
{
	if (!VRInput)
	{
		FORERUNNER_WARN(OpenVR, "Attempted to register action set before initialising");
		return InputBindingID();
	}

	vr::VRActionSetHandle_t NewSet;
	VRInput->GetActionSetHandle(("/actions/" + Set).c_str(), &NewSet);
	FORERUNNER_LOG(OpenVR, "Registered ActionSet {} with id {}", Set, NewSet);

	return NewSet;
}

void OpenVR::ActivateActionSet(InputBindingID ID)
{
	for (vr::VRActiveActionSet_t& Set : ActiveActionSets)
	{
		if (Set.ulActionSet == ID)
		{
			return;
		}
	}

	vr::VRActiveActionSet_t& NewSet = ActiveActionSets.emplace_back();
	NewSet.ulActionSet = ID;
	NewSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
}

void OpenVR::DeactivateActionSet(InputBindingID ID)
{
	std::erase_if(ActiveActionSets, [ID](vr::VRActiveActionSet_t& Set)
	{
		return Set.ulActionSet == ID;
	});
}
