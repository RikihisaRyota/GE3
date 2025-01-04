#pragma once
/**
 * @file ModelManager.h
 * @brief モデルの管理
 */
#include <filesystem>
#include <memory>
#include <vector>

#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/PipelineState.h"

#include "Engine/Animation/Animation.h"
#include "ModelHandle.h"
#include "Model.h"

class CommandContext;
struct WorldTransform;
struct ViewProjection;
class ModelManager {
public:
	// インスタンス
	static ModelManager* GetInstance();
	// パイプライン生成
	static void CreatePipeline(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
	// パイプライン破棄
	static void DestroyPipeline();

	// Load
	ModelHandle Load(const std::filesystem::path path);

	// Getter
	Model& GetModel(const ModelHandle& modelHandle) { return *models_[modelHandle.index_]; }

	// 描画
	void Draw(const Matrix4x4& worldMatrix, const ViewProjection& viewProjection, const ModelHandle& modelHandle, CommandContext& commandContext);
	void Draw(const Matrix4x4& worldMatrix, Animation::Animation& skinning,const ViewProjection& viewProjection, const ModelHandle& modelHandle, CommandContext& commandContext);
private:
	static std::unique_ptr<PipelineState> pipelineState_;
	static std::unique_ptr<RootSignature> rootSignature_;
	static std::unique_ptr<PipelineState> skinningPipelineState_;
	static std::unique_ptr<RootSignature> skinningRootSignature_;

	std::vector<std::unique_ptr<Model>> models_;
};