#include "DebugCamera.h"

#include <numbers>

#include "imgui.h"

#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Math/Vector2.h"

void DebugCamera::Update(ViewProjection* viewProjection) {
#ifdef ENABLE_IMGUI
	ImGui::Checkbox("isDebugCamara", &isDebugCamera_);
	ImGui::Text("DebugCameraPos\nX:%f,Y:%f,Z:%f", viewProjection->translation_.x, viewProjection->translation_.y, viewProjection->translation_.z);
#endif // ENABLE_IMGUI
		if (isDebugCamera_) {
			auto input = Engine::Input::GetInstance();
			int32_t wheel = input->GetWheel();
			Vector2 mouseMove = input->GetMouseMove();
			if (input->PushMouse(1)) {
				float rot = std::numbers::pi_v<float> / 180.0f;
				viewProjection->rotation_.x += rot * mouseMove.y * 0.1f;
				viewProjection->rotation_.y += rot * mouseMove.x * 0.1f;
			}
			else if (input->PushMouse(2)) {
				Matrix4x4 rotMat = MakeRotateXYZMatrix(viewProjection->rotation_);
				Vector3 cameraX = GetXAxis(rotMat) * static_cast<float>(-mouseMove.x) * 0.01f;
				Vector3 cameraY = GetYAxis(rotMat) * static_cast<float>(mouseMove.y) * 0.01f;
				viewProjection->translation_ += cameraX + cameraY;
			}
			else if (wheel != 0) {
				Matrix4x4 rotMat = MakeRotateXYZMatrix(viewProjection->rotation_);
				Vector3 cameraZ = GetZAxis(rotMat) * (static_cast<float>(wheel / 120.0f) * 0.5f);
				viewProjection->translation_ += cameraZ;
			}
			viewProjection->UpdateMatrix();
		}
}