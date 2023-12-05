#pragma once

#include <memory>

#include "../Game/GPUParticle.h"

class CommandContext;
class GameScene {
public:
	GameScene();
	void Initialize();
	void Update();
	void Draw(const CommandContext& commandContext);
private:
	GPUParticle gpuParticle_;
};