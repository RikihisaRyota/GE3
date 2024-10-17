#pragma once
#include "Engine/Math/Vector2.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "ColorBuffer.h"
#include "UploadBuffer.h"
#include "Engine/Graphics/GaussianFilter.h"

class CommandContext;
class ColorBuffer;
class Bloom {
public:
	struct Desc {
		float intensity = 1.0f;
	};

	void Initialize(const ColorBuffer& target);
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	void Debug();
private:
	static const uint32_t kMaxLevel = 2;
	RootSignature luminanceRootSignature_;
	PipelineState luminancePipelineState_;

	RootSignature bloomRootSignature_;
	PipelineState bloomPipelineState_;

	ColorBuffer originalBuffer_;
	ColorBuffer luminanceBuffer_;
	GaussianFilter gaussianFilter_[kMaxLevel];
	Desc desc_;
};