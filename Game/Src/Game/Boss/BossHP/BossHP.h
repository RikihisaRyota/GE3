#pragma once

#include <stdint.h>

// ボスHP管理
class BossHP {
public:
	BossHP();
	void Initialize();
	void Update();
	void DrawImGui();

	// ダメージ時に呼び出す
	void HitDamage(int32_t damege);
private:
	int32_t currentHP_;
	int32_t maxHP_;
};