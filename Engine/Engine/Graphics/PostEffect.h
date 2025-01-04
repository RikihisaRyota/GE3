#pragma once
/**
 * @file PostEffect.h
 * @brief PostEffect用
 */
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
	// 初期化
	void Initialize(const ColorBuffer& target);
	// 描画
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	// Setter/Getter
	void SetFlag(bool flag) { isUsed_ = flag; }
	bool GetFlag() { return isUsed_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;
	bool isUsed_;
};