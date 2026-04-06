#pragma once
#include <cstdint>
#include "../utils/matrices.h"

enum class EVR_Eye : uint8_t
{
	Left,
	Right
};

enum class EVR_Controller : uint8_t
{
	Left,
	Right
};

struct VR_Bounds
{
	float x;
	float y;
	float w;
	float h;
};

typedef uint64_t InputBindingID;

// Generic interface for interacting with a VR runtime, should be implemented for each vendor (OpenVR, OpenXR, etc) supported
class IVR
{
public:

	// Early initialisation, called near startup
	virtual bool EarlyInit() = 0;
	// Late initialisation, called once important systems such as directx have been initialised
	virtual bool Init() = 0;
	// Called when VR should end and be cleaned up gracefully
	virtual void Shutdown() = 0;
	// Called every frame
	virtual void Update(float DeltaTime) = 0;
	// Called when the game has a new frame to submit for an eye, ViewBounds defines which portion of the render target holds the current eye
	virtual void SubmitEye(EVR_Eye Eye, struct ID3D11Texture2D* Texture, const VR_Bounds& ViewBounds = VR_Bounds{ .x = 0.0f, .y = 0.0f, .w = 1.0f, .h = 1.0f}) = 0;

	virtual void SetDevice(struct ID3D11Device* Device) = 0;
	virtual void SetDeviceContext(struct ID3D11DeviceContext* Context) = 0;

	// Get the desired width of the render target for one eye
	virtual int32_t GetDesiredWidth() const = 0;
	// Get the desired height of the render target for one eye
	virtual int32_t GetDesiredHeight() const = 0;
	// Get the vertical field of view (in radians) the current eye should be rendered at
	virtual float GetVerticalFieldOfView(EVR_Eye Eye) const = 0;
	// Get the transform of the HMD, relative to the VR origin. Units are meters, coordinate system is forward = +x, right = -y, up = +z
	virtual Matrix4 GetHMDTransform() const = 0;
	// Get the transform of the requested eye, relative to the HMD. Units are meters, coordinate system is forward = +x, right = -y, up = +z
	virtual Matrix4 GetEyeTransform(EVR_Eye Eye) const = 0;
	// Get the transform of a controller, relative to the VR origin. Units are meters, coordinate system is forward = +x, right = -y, up = +z
	virtual Matrix4 GetControllerTransform(EVR_Controller Controller) const = 0;

	// Register a bool input (e.g. button) under an action set and get a binding ID which can be used to query the input state
	virtual InputBindingID RegisterBoolInput(const std::string& Set, const std::string& Action) = 0;
	// Register a 2d vector input (e.g. joystick) under an action set and get a binding ID which can be used to query the input state
	virtual InputBindingID RegisterVector2Input(const std::string& Set, const std::string& Action) = 0;
	// Get the current state of the bool input registered with the given ID, returns default value if the ID is invalid
	virtual bool GetBoolInput(InputBindingID ID) const = 0;
	// Get the current state of the bool input, and whether it has changed since last update, registered with the given ID. Returns default value if the ID is invalid
	virtual bool GetBoolInput(InputBindingID ID, bool& bHasChanged) const = 0;
	// Get the current state of a 2d vector input registered with the given ID. Returns default value if the ID is invalid
	virtual Vector2 GetVector2Input(InputBindingID ID) const = 0;
};



// Formatting
#include <format>
template <>
struct std::formatter<EVR_Eye> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(EVR_Eye e, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "{}", e == EVR_Eye::Left ? "Left" : "Right");
	}
};

template <>
struct std::formatter<EVR_Controller> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	auto format(EVR_Controller c, std::format_context& ctx) const
	{
		return std::format_to(ctx.out(), "{}", c == EVR_Controller::Left ? "Left" : "Right");
	}
};