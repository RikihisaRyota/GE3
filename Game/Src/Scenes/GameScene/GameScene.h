#pragma once

#include <memory>

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleEditor.h"
#include "Engine/DebugCamera/DebugCamera.h"
#include "Src/Game/DebugCamera/DebugCamera.h"
#include "Src/Game/Player/Player.h"
#include "Src/Game/FollowCamera/FollowCamera.h"
#include "Src/Scenes/BaseScene/BaseScene.h"

#include "Engine/Sprite/SpriteHandle.h"

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
	
	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<GPUParticleEditor> gpuParticleEditor_;
	std::unique_ptr<GPUParticleManager> gpuParticleManager_;
	std::unique_ptr<FollowCamera> followCamera_;
	std::unique_ptr<Player> player_;
	TextureHandle gpuTexture_;
	ModelHandle modelHandle_;
	ModelHandle terrainHandle_;
	WorldTransform worldTransform_;
	//int32_t soundHandle_;
	//int32_t playHandle_;
	Vector4 color_;
};