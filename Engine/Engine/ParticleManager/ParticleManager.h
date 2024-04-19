#pragma once

#include <d3dx12.h>
#include <vector>

#include "Engine/Graphics/DescriptorHandle.h"
#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Texture/TextureHandle.h"
#include "ParticleShaderStruct.h"
#include "Particle/Particle.h"

class CommandContext;
class ParticleManager {
public:
	struct Instancing {
		TextureHandle textureHandle;
		Particle* particle;
		uint32_t maxInstance = 1000;
		uint32_t currentInstance;
		// ワールドトランスフォームマトリックスリソース
		UploadBuffer instancingBuff;
		// ワールドトランスフォーム
		CPUParticleShaderStructs::ParticleForGPU* instancingDate = nullptr;
		DescriptorHandle descriptorHandle;
		bool isAlive;
	};

	struct cMaterial {
		Vector4 color;
	};
public:
	static ParticleManager* GetInstance();
	void Initialize();
	void Update();
	void Draw(CommandContext& commandContext, const ViewProjection& viewProjection);
	void Shutdown();
	void AddParticle(CPUParticleShaderStructs::Emitter* emitter, CPUParticleShaderStructs::ParticleMotion* particleMotion, TextureHandle textureHandle);
private:
	static const size_t kNumInstancing = 100;
	static bool CompareParticles(const Instancing* a, const Instancing* b) {
		return a->isAlive > b->isAlive;
	}
#pragma region DirectX関連
	// グラフィックパイプライン
	PipelineState graphicsPipeline_;
	RootSignature rootSignature_;
#pragma region 頂点バッファ
	// 頂点バッファ
	UploadBuffer vertBuff_;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
#pragma endregion
#pragma region インデックスバッファ
	// インデックスバッファ
	UploadBuffer idxBuff_;
	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	// 頂点インデックスデータ
	std::vector<uint16_t> indices_;
#pragma endregion
#pragma region 
	std::vector<Instancing*> instancing_;
	// マテリアルリソース
	UploadBuffer materialBuff_;
	// マテリアル
	cMaterial* material_ = nullptr;
#pragma endregion

};

