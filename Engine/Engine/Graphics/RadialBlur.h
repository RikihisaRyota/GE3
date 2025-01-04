#pragma once
/**
 * @file RadialBlur.h
 * @brief ポストエフェクト用
 */
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
	// 初期化
	void Initialize(const ColorBuffer& target);
	// 描画
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	// Debug
	void DrawImGui();

	// Setter/Getter
	Desc& GetDesc() { return desc_; }
	void SetFlag(bool flag) { isUsed_ = flag; }
	bool GetFlag() { return isUsed_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;

	UploadBuffer descBuffer_;
	Desc desc_;
	bool isUsed_;
};