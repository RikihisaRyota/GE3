#pragma once

#include <array>
#include <memory>
#include <vector>
#include <cstdint>

#include <wrl.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/DescriptorHandle.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"


struct ViewProjection;
class GPUParticle {
private:
	struct Vertex {
		Vector3 position;
		Vector2 texcoord;
	};
	struct Particle {
		Vector3 scale; // 12
		float pad1;	// 16
		Vector3 velocity; // 28
		float pad2;	// 32
		Vector3 rotate;	// 44
		float pad3;	// 48
		Vector3 translate; // 60
		float pad4;	// 64
		uint32_t isAlive;	// 68
		float pad5[3];	// 80
		uint32_t isHit;	// 84
		float pad6[3];	// 96
		float aliveTime;	// 100
		float pad7[3];	// 112
	};
	struct ParticleInfo {
		float speed;
	};

	struct IndirectCommand {
		D3D12_GPU_VIRTUAL_ADDRESS srv;
		D3D12_DRAW_INDEXED_ARGUMENTS drawIndex;
	};
	
	struct ParticleArea {
		Vector3 min;
		float pad1;
		Vector3 max;
		float pad2;
	};
	// ボールデータ
	struct BallBufferData {
		Vector3 position;
		float size;
	};
	// ボール
	struct Ball {
		Vector3 position;
		Vector3 velocity;
		float size;
		bool isAlive;
		float aliveTime;
	};
	// ボールカウント
	struct BallCount {
		int ballCount;
	};
public:
	GPUParticle();
	~GPUParticle();
	void Initialize();
	void Update(ViewProjection* viewProjection);
	void Render(const ViewProjection& viewProjection);

private:
	static const UINT kNumThread;
	static const UINT CommandSizePerFrame;
	static const UINT CommandBufferCounterOffset;
	static const UINT kMaxBall = 15;
	static const UINT kAliveTime = 100;
	static const UINT ComputeThreadBlockSize = 128;
	static const UINT kShotCoolTime = 10;

	void InitializeSpawnParticle();
	void InitializeUpdateParticle();
	void InitializeParticleArea();
	void InitializeGraphics();
	void InitializeBall();

	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> spawnComputeRootSignature_;
	std::unique_ptr<PipelineState> spawnComputePipelineState_;
	std::unique_ptr<RootSignature> updateComputeRootSignature_;
	std::unique_ptr<PipelineState> updateComputePipelineState_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	std::vector<Vertex>	vertices_;

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	std::vector<uint16_t> indices_;

	GpuResource rwStructuredBuffer_;
	DescriptorHandle rwStructuredBufferHandle_;
	// UAVハンドル
	DescriptorHandle uavHandle_;
	
	// モデル
	ModelHandle gpuParticleModelHandle_;
	WorldTransform worldTransform_;

	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_;
	GpuResource commandBuffer_;
	UploadBuffer commandUploadBuffer_;
	DescriptorHandle commandHandle_;
	// 計算結果格納用
	GpuResource processedCommandBuffers_;
	DescriptorHandle processedCommandsHandle_;
	GpuResource processedCommandBufferCounterReset_;
	GpuResource drawArgumentBuffer_;
	DescriptorHandle drawArgumentHandle_;

	ParticleInfo* particleInfo_;
	Particle* particle_;
	UploadBuffer updateConstantBuffer_;

	// パーティクルのエリア
	ParticleArea* particleArea_;
	UploadBuffer particleAreaBuffer_;
	// ボールバッファー
	BallBufferData* ball_;
	UploadBuffer ballBuffer_;
	DescriptorHandle ballBufferHandle_;
	// ボールカウント
	BallCount* ballCount_;
	UploadBuffer ballCountBuffer_;
	// ボールデータ
	std::array<Ball, kMaxBall> ballData_;

	ModelHandle ballModelHandle_;
	std::array<WorldTransform, kMaxBall> ballWorldTransform_;
	
	// テクスチャ
	UINT shotTime;

	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
};