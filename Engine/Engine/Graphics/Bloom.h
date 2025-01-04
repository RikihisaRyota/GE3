#pragma once
/**
 * @file Bloom.h
 * @brief ポストエフェクトでブルームをかける際に仕様
 */
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
		float intensity = 0.5f;
	};
	// 初期化
	void Initialize(const ColorBuffer& target);
	// 描画
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	// デバック
	void Debug();
private:
	static const uint32_t kMaxLevel = 4;
	RootSignature luminanceRootSignature_;
	PipelineState luminancePipelineState_;

	RootSignature bloomRootSignature_;
	PipelineState bloomPipelineState_;

	ColorBuffer originalBuffer_;
	ColorBuffer luminanceBuffer_;
	GaussianFilter gaussianFilter_[kMaxLevel];
	Desc desc_;
	bool isUsed_;
};