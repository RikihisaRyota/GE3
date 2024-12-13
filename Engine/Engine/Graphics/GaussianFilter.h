#pragma once
/**
 * @file GaussinFilter.h
 * @brief ポストエフェクトを実装するために
 */
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
	void Render(CommandContext& commandContext, ColorBuffer& target);
	ColorBuffer& GetColorBuffer() { return verticalBuffer_; }
private:
	RootSignature rootSignature_;
	PipelineState horizontalPipelineState_;
	PipelineState verticalPipelineState_;
	ColorBuffer horizontalBuffer_;
	ColorBuffer verticalBuffer_;
	ColorBuffer originalTexture_;
};