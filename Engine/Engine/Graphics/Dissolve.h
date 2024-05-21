#pragma once

#include "RootSignature.h"
#include "PipelineState.h"
#include "UploadBuffer.h"
#include "ColorBuffer.h"
#include "Engine/Texture/TextureHandle.h"

class CommandContext;
class DepthBuffer;
class Dissolve {
public:
	enum RootParameter {
		kTexture,
		kMaskTexture,
		kThreshold,
		kCount
	};

	struct Desc {
		float threshold;
		float edgeWidth;
	};

	void Initialize(const ColorBuffer& target);
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	void DrawImGui();
	Desc& GetDesc() { return desc_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;
	TextureHandle maskTextureHandle_;
	UploadBuffer thresholdBuffer_;

	Desc desc_;
};