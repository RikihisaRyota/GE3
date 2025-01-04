#pragma once
/**
 * @file HSVFilter.h
 * @brief ポストエフェクト用
 */
#include "RootSignature.h"
#include "PipelineState.h"
#include "ColorBuffer.h"

class CommandContext;
class ColorBuffer;
class HSVFilter {
public:
	struct Desc {
		float hue;
		float saturation;
		float value;
 	};
	enum RootParameter {
		kTexture,
		kHSV,
		kCount
	};
	// 初期化
	void Initialize(const ColorBuffer& target);
	// 描画
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	// Getter
	Desc& GetDesc() { return desc_; }
	// Debug
	void Debug();
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;
	Desc desc_;
};