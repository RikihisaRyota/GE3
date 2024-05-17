#pragma once

#include "RootSignature.h"
#include "PipelineState.h"
#include "ColorBuffer.h"
#include "UploadBuffer.h"

class CommandContext;

class PostEffect {
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