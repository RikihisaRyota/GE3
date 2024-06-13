#pragma once

#include <stdint.h>

class BossHP {
public:
	void Initialize();
	void Update();
	void DrawImGui();
private:
	int32_t currentHP_;
	int32_t maxHP_;
};