#include "Lighting.h"

#include "Engine/Math/MyMath.h"

#include "imgui.h"

Lighting* Lighting::GetInstance() {
	static Lighting lighting;
	return &lighting;
}

void Lighting::Initialize() {
	directionLightBuffer_.Create(L"DirectionLightBuffer", sizeof(DirectionLight));
	directionLightData_ = new DirectionLight();
	directionLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	directionLightData_->direction = { 0.0f,-1.0f,0.0f };
	directionLightData_->intensity = 1.0f;
	directionLightData_->sharpness = 1.0f;
	directionLightBuffer_.Copy(directionLightData_, sizeof(DirectionLight));

	pointLightBuffer_.Create(L"PointLightBuffer", sizeof(PointLight));
	pointLightData_ = new PointLight();
	pointLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData_->position = { 0.0f,6.0f,0.0f };
	pointLightData_->intensity = 1.0f;
	pointLightData_->radius = 5.0f;
	pointLightData_->decay = 1.0f;
	pointLightData_->sharpness = 1.0f;
	pointLightBuffer_.Copy(pointLightData_, sizeof(PointLight));
}

void Lighting::Update() {
#ifdef ENABLE_IMGUI
	ImGui::Begin("Lighting");
	if (ImGui::TreeNode("DirectionLight")) {
		ImGui::DragFloat4("Color", &directionLightData_->color.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat3("Direction", &directionLightData_->direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Intensity", &directionLightData_->intensity, 0.1f, 0.0f);
		ImGui::DragFloat("Sharpness", &directionLightData_->sharpness, 0.1f, 0.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("PointLight")) {
		ImGui::DragFloat4("Color", &pointLightData_->color.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat3("Position", &pointLightData_->position.x, 0.01f);
		ImGui::DragFloat("Intensity", &pointLightData_->intensity, 0.1f,0.0f);
		ImGui::DragFloat("Radius", &pointLightData_->radius, 0.1f,0.0f);
		ImGui::DragFloat("Decay", &pointLightData_->decay, 0.1f,0.0f);
		ImGui::DragFloat("Sharpness", &pointLightData_->sharpness, 0.1f, 0.0f);
		ImGui::TreePop();
	}
	ImGui::End();
#endif // ENABLE_IMGUI
	directionLightData_->direction.Normalize();
	directionLightBuffer_.Copy(directionLightData_, sizeof(DirectionLight));
	pointLightBuffer_.Copy(pointLightData_, sizeof(PointLight));
}
