#pragma once
#include <filesystem>
#include <memory>
#include <vector>

#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/PipelineState.h"

#include "Sprite.h"
#include "SpriteHandle.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Texture/Texture.h"

class CommandContext;
class SpriteManager {
public:
	static SpriteManager* GetInstance();
	static void CreatePipeline(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
	static void DestroyPipeline();

	SpriteHandle Create(
		TextureHandle textureHandle, Vector2 position, Vector2 anchorpoint = { 0.0f, 0.0f },Vector4 color = { 1, 1, 1, 1 }, bool isFlipX = false, bool isFlipY = false);

	Sprite* GetSprite(const SpriteHandle& spriteHandle) { return sprites_[spriteHandle].get(); }

	void Draw(const SpriteHandle& spriteHandle, CommandContext& commandContext);
private:
	static std::unique_ptr<PipelineState> pipelineState_;
	static std::unique_ptr<RootSignature> rootSignature_;

	std::vector<std::unique_ptr<Sprite>> sprites_;
};