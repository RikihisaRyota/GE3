#pragma once

#include <memory>
#include <vector>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/ViewProjection.h"

class CommandContext;
class DrawLine {
private:
	struct Vertex {
		Vector3 position;
		Vector4 color;
	};
public:
	static DrawLine* GetInstance();
	void Initialize();
	void Draw(CommandContext& commandContext, const ViewProjection& viewProjection);
	void Reset();
	
	void SetLine(const Vector3& start,const Vector3& end,const Vector4& color);
	void SetLine(const Vector3& line,const Vector4& color);
private:
	static const UINT kMaxLineCount = 1 << 15;
	
	void CreateRootSignature();
	void CreatePipelineState();
	
	void CreateVertexBuffer();

	std::unique_ptr<PipelineState> pipelineState_;
	std::unique_ptr<RootSignature> rootSignature_;

	// 頂点バッファ
	UploadBuffer vertBuff_;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	// 頂点データ配列
	std::vector<Vertex> vertices_;
};