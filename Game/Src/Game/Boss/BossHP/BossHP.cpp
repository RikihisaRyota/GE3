#include "BossHP.h"

#include <algorithm>

#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"
void BossHP::Initialize() {
	JSON_OPEN("Resources/Data/Boss/BossState.json");
	JSON_OBJECT("StateRoot");
	JSON_LOAD(maxHP_);
	JSON_ROOT();
	JSON_CLOSE();
}

void BossHP::Update() {
	currentHP_ = std::clamp(currentHP_, 0, maxHP_);
}

void BossHP::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("BossHP")) {
		int maxHP = maxHP_;

		ImGui::DragInt("MaxHP", &maxHP, 1, 0);
			ImGui::EndMenu();
	}
	ImGui::End();
}
