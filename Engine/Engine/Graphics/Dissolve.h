#pragma once
/**
 * @file Dissolve.h
 * @brief ポストエフェクトでDissolveを使用するため
 */
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
	TextureHandle maskTextureHandle_;
	UploadBuffer thresholdBuffer_;

	Desc desc_;
	bool isUsed_;
};