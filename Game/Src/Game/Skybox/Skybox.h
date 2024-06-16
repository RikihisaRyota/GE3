#pragma once
#include <vector>


#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Texture/TextureHandle.h"


class CommandContext;
struct ViewProjection;
class Skybox {
public:
	Skybox();
	void Initialize();
	void Update();
	void Draw(CommandContext& commandContext, const ViewProjection& viewProjection);

	void DrawImGui();
private:
	std::unique_ptr<PipelineState> pipelineState_;
	std::unique_ptr<RootSignature> rootSignature_;

	// 頂点バッファ
	UploadBuffer vertBuff_;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	WorldTransform worldTransform_;

	std::vector<Vector4> vertices_;

	TextureHandle textureHandle_;
};