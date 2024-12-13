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

	void Initialize(const ColorBuffer& target);
	void Render(CommandContext& commandContext, ColorBuffer& texture);
	void SetFlag(bool flag) { isUsed_ = flag; }
	bool GetFlag() { return isUsed_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;
	bool isUsed_;
};