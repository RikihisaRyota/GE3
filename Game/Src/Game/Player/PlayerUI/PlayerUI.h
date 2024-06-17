#pragma once

#include "Engine/Sprite/SpriteHandle.h"

class CommandContext;
class PlayerUI {
public:
	PlayerUI();
	void Initialize();
	void Update();
	void Draw(CommandContext& commandContext);
private:
	SpriteHandle reticleUIHandle_;

};