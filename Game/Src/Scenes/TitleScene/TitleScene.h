#pragma once

#include <memory>

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/DebugCamera/DebugCamera.h"
#include "Engine/Math/ViewProjection.h"
#include "Src/Scenes/BaseScene/BaseScene.h"

#include "Engine/Sprite/SpriteHandle.h"

class CommandContext;
class TitleScene : public BaseScene {
public:
	TitleScene();
	~TitleScene();
	void Initialize() override;
	void Update(CommandContext& commandContext) override;
	void Draw(CommandContext& commandContext) override;
	void Finalize() override;
private:
	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<GPUParticleManager> gpuParticleManager_;
	TextureHandle gpuTexture_;
	ViewProjection viewProjection_;

	TextureHandle titleTexture_;
	SpriteHandle titleHandle_;
};
