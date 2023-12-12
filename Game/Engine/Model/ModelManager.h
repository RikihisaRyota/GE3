#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/PipelineState.h"

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
private:
	struct RenderModel {
		uint32_t modelHandle;
		uint32_t modelCount;
	};

	static std::unique_ptr<PipelineState> pipelineState_;
	static std::unique_ptr<RootSignature> rootSignature_;

	std::vector<std::unique_ptr<Model>> models_;
	std::vector<RenderModel> renderModels_;
};