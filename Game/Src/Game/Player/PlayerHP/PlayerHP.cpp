#include "PlayerHP.h"

#include <algorithm>

#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"
PlayerHP::PlayerHP() {
	JSON_OPEN("Resources/Data/Player/playerHP.json");
	JSON_OBJECT("PlayerHP");
	JSON_LOAD(maxHP_);
	JSON_ROOT();
	JSON_CLOSE();
}

void PlayerHP::Initialize() {
	currentHP_ = maxHP_;
}

void PlayerHP::Update() {
	currentHP_ = std::clamp(currentHP_, 0, maxHP_);
}

void PlayerHP::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Player")) {
		if (ImGui::TreeNode("PlayerHP")) {
			int maxHP = maxHP_;
			ImGui::DragInt("MaxHP", &maxHP, 1, 0);
			maxHP_ = maxHP;
			int currentHP = currentHP_;
			ImGui::DragInt("CurrentHP", &currentHP, 1, 0, maxHP);
			currentHP_ = currentHP;
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/playerHP.json");
				JSON_OBJECT("PlayerHP");
				JSON_SAVE(maxHP_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
}

void PlayerHP::HitDamage(int32_t damage) {
	currentHP_ -= damage;
	currentHP_ = std::clamp(currentHP_, 0, maxHP_);
}
