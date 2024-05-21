#pragma once

#include "Engine/Math/Vector2.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "ColorBuffer.h"
#include "UploadBuffer.h"

class CommandContext;
class ColorBuffer;
class RadialBlur {
public:
	enum RootParameter {
		kTexture,
		kDesc,
		kCount
	};

	struct Desc {
		Vector2 center;
		float blurWidth;
		float pad;
	};

	void Initialize(const ColorBuffer& target);
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	void DrawImGui();

	Desc& GetDesc() { return desc_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;

	UploadBuffer descBuffer_;
	Desc desc_;
};