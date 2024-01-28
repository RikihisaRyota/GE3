#pragma once

#include <memory>

#include "Engine/GPUParticleManager/GPUParticleManager.h"
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

	std::unique_ptr<GPUParticleManager> gpuParticleManager_;
	std::unique_ptr<DebugCamera> debugCamera_;
	TextureHandle gpuTexture_;
	ModelHandle modelHandle_;
	ModelHandle terrainHandle_;
	WorldTransform worldTransform_;
	int32_t soundHandle_;
	int32_t playHandle_;
	Vector4 color_;
};