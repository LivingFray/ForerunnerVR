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

	// TODO: Create overlay here

	std::filesystem::path Manifest = ForerunnerPath / "VR" / "OpenVR" / "forerunner.vrmanifest";
	vr::EVRApplicationError AppErr = vr::VRApplications()->AddApplicationManifest(Manifest.string().c_str());

	if (AppErr != vr::VRApplicationError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not add application manifest: {}", static_cast<int32_t>(AppErr));
	}

	AppErr = vr::VRApplications()->IdentifyApplication(GetCurrentProcessId(), "livingfray.forerunner");

	if (AppErr != vr::VRApplicationError_None)
	{
		FORERUNNER_WARN(OpenVR, "Could not set id: {}", static_cast<int32_t>(AppErr));
	}

	// TODO: Load actions here (could also be done later I suppose)

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
		FORERUNNER_WARN(OpenVR, "Could not submit {} eye texture : {}", Eye, static_cast<int>(Error));
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
