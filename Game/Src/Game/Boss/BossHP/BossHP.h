#pragma once

#include <stdint.h>

class BossHP {
public:
	BossHP();
	void Initialize();
	void Update();
	void DrawImGui();

	void HitDamage(int32_t damege);
private:
	int32_t currentHP_;
	int32_t maxHP_;
};