#include "BossHP.h"

#include <algorithm>

#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"
BossHP::BossHP() {
	JSON_OPEN("Resources/Data/Boss/BossState.json");
	JSON_OBJECT("StateRoot");
	JSON_LOAD(maxHP_);
	JSON_ROOT();
	JSON_CLOSE();
}
void BossHP::Initialize() {
	currentHP_ = maxHP_;
}

void BossHP::Update() {
	currentHP_ = std::clamp(currentHP_, 0, maxHP_);
}

void BossHP::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		if (ImGui::TreeNode("BossHP")) {
			int maxHP = maxHP_;
			ImGui::DragInt("MaxHP", &maxHP, 1, 0);
			maxHP_ = maxHP;
			int currentHP = currentHP_;
			ImGui::DragInt("CurrentHP", &currentHP, 1, 0, maxHP);
			currentHP_ = currentHP;
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/BossState.json");
				JSON_OBJECT("StateRoot");
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

void BossHP::HitDamage(int32_t damage) {
	currentHP_ -= damage;
	currentHP_ = std::clamp(currentHP_, 0, maxHP_);
}
