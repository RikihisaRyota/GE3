#pragma once

#include <stdint.h>

class PlayerHP {
public:
	PlayerHP();
	void Initialize();
	void Update();
	void DrawImGui();

	void HitDamage(int32_t damage);
private:
	int32_t currentHP_;
	int32_t maxHP_;
};