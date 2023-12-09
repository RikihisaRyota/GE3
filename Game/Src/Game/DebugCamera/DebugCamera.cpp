#include "DebugCamera.h"

#include "../../Game/Engine/ImGui/ImGuiManager.h"
#include "../../Game/Engine/Input/Input.h"
#include "../../Game/Engine/Math/MyMath.h"
#include "../../Game/Engine/Math/Vector2.h"


DebugCamera::DebugCamera() {}

void DebugCamera::Update(ViewProjection* viewProjection) {
	Input* input = Input::GetInstance();
	int32_t wheel = input->GetWheel();
	Vector2 mouseMove = input->GetMouseMove();
	if (input->PushMouse(1)) {
		float rot = static_cast<float>(pi / 180.0f);
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