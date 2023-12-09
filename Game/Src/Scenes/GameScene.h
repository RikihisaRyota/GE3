#pragma once

#include <memory>

#include "../Game/GPUParticle/GPUParticle.h"
#include "../Game/DebugCamera/DebugCamera.h"


class CommandContext;
class GameScene {
public:
	GameScene();
	void Initialize();
	void Update();
	void Draw(const CommandContext& commandContext);
private:
	ViewProjection viewProjection_;

	std::unique_ptr<GPUParticle> gpuParticle_;
	std::unique_ptr<DebugCamera> debugCamera_;

};