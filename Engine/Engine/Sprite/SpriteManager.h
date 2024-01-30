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
struct ViewProjection;
class SpriteManager {
public:
	static SpriteManager* GetInstance();
	static void CreatePipeline(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
	static void DestroyPipeline();

	SpriteHandle Load(const std::filesystem::path path);

	//Texture& GetTexture(const TextureHandle& textureHandle) { return *sprite_[textureHandle]; }

	void Draw(const Vector2& pos, const SpriteHandle& spriteHandle, CommandContext& commandContext);
private:
	void CreateIndexVertexBuffer();
	static std::unique_ptr<PipelineState> pipelineState_;
	static std::unique_ptr<RootSignature> rootSignature_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	std::vector<std::unique_ptr<Sprite>> sprites_;
};