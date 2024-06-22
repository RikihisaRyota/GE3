#pragma once

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
	static ModelManager* GetInstance();
	static void CreatePipeline(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
	static void DestroyPipeline();

	ModelHandle Load(const std::filesystem::path path);

	Model& GetModel(const ModelHandle& modelHandle) { return *models_[modelHandle.index_]; }

	void Draw(const WorldTransform& worldTransform,const ViewProjection& viewProjection, const ModelHandle& modelHandle, CommandContext& commandContext);
	void Draw(const WorldTransform& worldTransform, Animation::Animation& skinning,const ViewProjection& viewProjection, const ModelHandle& modelHandle, CommandContext& commandContext);
private:
	static std::unique_ptr<PipelineState> pipelineState_;
	static std::unique_ptr<RootSignature> rootSignature_;
	static std::unique_ptr<PipelineState> skinningPipelineState_;
	static std::unique_ptr<RootSignature> skinningRootSignature_;

	std::vector<std::unique_ptr<Model>> models_;
};