#include "uicomponent.h"
#include "deltamodule.h"
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/render/render_cameras.h"

#include "DirectXMath.h"

void UIComponent::UpdateHUD()
{
	real_point3d camPos = g_render_camera().position;
	real_vector3d camFwd = g_render_camera().forward;
	real_vector3d camUp = g_render_camera().up;

	// Position in front of the camera
	DirectX::XMFLOAT3 pos(camPos.x + camFwd.x * UIDistance, camPos.y + camFwd.y * UIDistance, camPos.z + camFwd.z * UIDistance);

	// Create a billboard matrix that faces toward the camera
	// The quad normal should point back toward the camera (opposite of forward)
	DirectX::XMFLOAT3 xmCamPos(-camFwd.x, -camFwd.y, -camFwd.z);
	DirectX::XMVECTOR forward = DirectX::XMLoadFloat3(&xmCamPos);
	DirectX::XMFLOAT3 xmCamUp(camUp.x, camUp.y, camUp.z);
	DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&xmCamUp);
	DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pos);

	// Create a view-like matrix but inverted so the quad faces the camera
	// We negate forward to point the quad normal toward the camera
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(up, forward);
	right = DirectX::XMVector3Normalize(right);
	DirectX::XMVECTOR correctedUp = DirectX::XMVector3Cross(forward, right);
	correctedUp = DirectX::XMVector3Normalize(correctedUp);

	// Build rotation matrix with right, up, and forward as basis vectors
	DirectX::XMMATRIX rot = DirectX::XMMATRIX(
		DirectX::XMVectorSetW(right, 0.0f),
		DirectX::XMVectorSetW(correctedUp, 0.0f),
		DirectX::XMVectorSetW(forward, 0.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)
	);

	DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

	// Scale the UI quad, accounting for aspect ratio
	// UI target is 800x600, so aspect ratio is 4:3
	float uiWidth = static_cast<float>(DeltaModule::Get().Render.GetUIWidth());
	float uiHeight = static_cast<float>(DeltaModule::Get().Render.GetUIHeight());
	float aspectRatio = uiWidth / uiHeight;
	
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.5f * aspectRatio, 0.5f, 0.5f);

	DeltaModule::Get().Render.SetUITransform(scale * rot * trans);
}
