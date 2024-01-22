#pragma once

#include <memory>

#include "Src/Game/GPUParticle/GPUParticle.h"
#include "Src/Game/DebugCamera/DebugCamera.h"
#include "Src/Scenes/BaseScene/BaseScene.h"

class CommandContext;
class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene();
	void Initialize() override;
	void Update() override;
	void Draw(CommandContext& commandContext) override;
	void Finalize() override;
private:
	ViewProjection viewProjection_;

	std::unique_ptr<GPUParticle> gpuParticle_;
	std::unique_ptr<DebugCamera> debugCamera_;
	ModelHandle modelHandle_;
	WorldTransform worldTransform_;
	int32_t soundHandle_;
	int32_t playHandle_;
};