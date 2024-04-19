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
	ColorBuffer* sourceTexture_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};
};