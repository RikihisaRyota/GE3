#pragma once

#include "RootSignature.h"
#include "PipelineState.h"
#include "ColorBuffer.h"

class CommandContext;
class ColorBuffer;
class GaussianFilter {
public:
	enum RootParameter {
		kTexture,
		kCount
	};

	void Initialize(const ColorBuffer& target);
	void Render(CommandContext& commandContext, ColorBuffer& texture);
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;
};