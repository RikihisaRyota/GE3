#pragma once

#include <stdint.h>

class PlayerHP {
public:
	// コンストラクタ
	PlayerHP();
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// デバック 
	void DrawImGui();
	// ダメージヒット時呼び出す
	void HitDamage(int32_t damage);
private:
	int32_t currentHP_;
	int32_t maxHP_;
};