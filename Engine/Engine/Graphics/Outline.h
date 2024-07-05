#pragma once

#include "RootSignature.h"
#include "PipelineState.h"
#include "UploadBuffer.h"
#include "ColorBuffer.h"

class CommandContext;
class DepthBuffer;
struct ViewProjection;
class Outline {
public:
	enum RootParameter {
		kInverseCamera,
		kTexture,
		kDepthTexture,
		kCount
	};

	void Initialize(const ColorBuffer& target);
	void Render(CommandContext& commandContext,ColorBuffer& texture,DepthBuffer& depth ,const ViewProjection& viewProjection);
	void SetFlag(bool flag) { isUsed_ = flag; }
	bool GetFlag() { return isUsed_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;

	UploadBuffer inverseCameraBuffer_;
	bool isUsed_;
};