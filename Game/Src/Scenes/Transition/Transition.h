#pragma once

#include <memory>

#include "Engine/Sprite/SpriteHandle.h"

class CommandContext;
class Transition {
public:
	Transition();

	void Initialize();

	void Update();

	void Draw(CommandContext& commandContext);

	bool GetIsSceneChange() { return isSceneChange_; }
	bool GetIsTransition() { return isTransition_; }
	bool GetIsSceneStart() { return isSceneStart_; }
private:
	static const float kMaxTime;
	bool isSceneChange_;
	bool isTransition_;
	bool isSceneStart_;
	float time_;
	SpriteHandle spriteHandle_;
};