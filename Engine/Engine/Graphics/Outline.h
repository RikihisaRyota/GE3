#pragma once
/**
 * @file Outline.h
 * @brief ポストエフェクト用
 */
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
	// 初期化
	void Initialize(const ColorBuffer& target);
	//　描画
	void Render(CommandContext& commandContext,ColorBuffer& texture,DepthBuffer& depth ,const ViewProjection& viewProjection);
	// Setter/Getter
	void SetFlag(bool flag) { isUsed_ = flag; }
	bool GetFlag() { return isUsed_; }
private:
	RootSignature rootSignature_;
	PipelineState pipelineState_;
	ColorBuffer temporaryBuffer_;

	UploadBuffer inverseCameraBuffer_;
	bool isUsed_;
};