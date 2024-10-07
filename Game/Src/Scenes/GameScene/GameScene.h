#pragma once

#include <vector>
#include <memory>

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleEditor.h"
#include "Engine/DebugCamera/DebugCamera.h"
#include "Src/Game/DebugCamera/DebugCamera.h"
#include "Src/Game/Boss/Boss.h"
#include "Src/Game/FollowCamera/FollowCamera.h"
#include "Src/Game/Player/Player.h"
#include "Src/Game/Skybox/Skybox.h"
#include "Src/Game/GameObject/GameObject.h"
#include "Src/Scenes/BaseScene/BaseScene.h"

#include "Engine/Sprite/SpriteHandle.h"
#include "Engine/Animation/Animation.h"
#include "Engine/Animation/Skeleton.h"
#include "Engine/Animation/Skinning.h"

class CommandContext;
class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene();
	void Initialize() override;
	void Update(CommandContext& commandContext) override;
	void Draw(CommandContext& commandContext) override;
	void Finalize() override;

private:

	std::unique_ptr<DebugCamera> debugCamera_;
	//std::unique_ptr<GPUParticleEditor> gpuParticleEditor_;
	std::unique_ptr<GPUParticleManager> gpuParticleManager_;
	std::unique_ptr<FollowCamera> followCamera_;

	std::unique_ptr<Player> player_;
	std::unique_ptr<Boss> boss_;
	std::unique_ptr<Skybox> skybox_;
	std::vector<std::unique_ptr<GameObject> >gameObject_;

	TextureHandle gpuTexture_;
	int32_t soundHandle_;
	int32_t playHandle_;

	GPUParticleShaderStructs::EmitterForCPU postEmitter_;
	GPUParticleShaderStructs::EmitterForCPU groundEmitter_;
#pragma region Debug
	//ModelHandle testModel_;
	//GPUParticleShaderStructs::VertexEmitterForCPU testVertexEmitter_;

	GPUParticleShaderStructs::EmitterForCPU testEmitter_;
	GPUParticleShaderStructs::EmitterForCPU test1Emitter_;
	GPUParticleShaderStructs::EmitterForCPU test2Emitter_;
	WorldTransform testWorldTransform_;
	GPUParticleShaderStructs::FieldForCPU testField_;
#pragma endregion
};