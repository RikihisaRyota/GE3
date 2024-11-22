#include "ParticleManager.h"

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Math/Vector2.h"


namespace Parameter {
	enum RootParameter {
		WorldTransform,
		ViewProjection,
		Material,
		Texture,
		Sampler,
		Count,
	};
}

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize() {
	// ルートシグネイチャー
	{
		CD3DX12_DESCRIPTOR_RANGE structureRange[1]{};
		structureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE range[1]{};
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[Parameter::RootParameter::Count]{};
		rootParameters[Parameter::RootParameter::WorldTransform].InitAsDescriptorTable(_countof(structureRange), structureRange);
		rootParameters[Parameter::RootParameter::ViewProjection].InitAsConstantBufferView(0);
		rootParameters[Parameter::RootParameter::Material].InitAsConstantBufferView(1);
		rootParameters[Parameter::RootParameter::Texture].InitAsDescriptorTable(_countof(range), range);
		rootParameters[Parameter::RootParameter::Sampler].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignature_.Create(L"Model RootSignature", desc);
	}
	// パイプライン生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = rootSignature_;

	D3D12_INPUT_ELEMENT_DESC inputElements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElements;
	inputLayoutDesc.NumElements = _countof(inputElements);
	desc.InputLayout = inputLayoutDesc;

	auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Particle/Particle.VS.hlsl", L"vs_6_0");
	auto ps = ShaderCompiler::Compile(L"Resources/Shaders/Particle/Particle.PS.hlsl", L"ps_6_0");

	desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
	desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
	desc.BlendState = Helper::BlendAlpha;
	desc.DepthStencilState = Helper::DepthStateRead;
	desc.RasterizerState = Helper::RasterizerDefault;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = RenderManager::GetInstance()->GetRenderTargetFormat();
	desc.DSVFormat = RenderManager::GetInstance()->GetDepthFormat();
	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.SampleDesc.Count = 1;
	graphicsPipeline_.Create(L"ParticleGraphicsPipeline", desc);
	HRESULT result = S_FALSE;

	struct VertexPos {
		Vector3 pos;
		Vector2 uv;
	};
	std::vector<VertexPos >vertices = {
		//	x      y     z      u     v
		{{-1.0f, -1.0f, 0.0f},{0.0f, 1.0f}}, // 左下 0
		{{-1.0f, +1.0f, 0.0f},{0.0f, 0.0f}}, // 左上 1
		{{+1.0f, +1.0f, 0.0f},{1.0f, 0.0f}}, // 右上 2
		{{+1.0f, -1.0f, 0.0f},{1.0f, 1.0f}}, // 右下 3
	};
	// 頂点インデックスの設定
	indices_ = { 0, 1, 2,
				 0, 2, 3,
	};
#pragma region 頂点バッファ
	// 頂点データのサイズ
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPos) * vertices.size());
	vertBuff_.Create(L"ParticleManagerVertBuff", sizeVB);
	vertBuff_.Copy(vertices.data(), sizeVB);
	// 頂点バッファビューの作成
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = sizeVB;
	vbView_.StrideInBytes = sizeof(VertexPos);
#pragma endregion 頂点バッファ
#pragma region インデックスバッファ
	// インデックスデータのサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices_.size());
	idxBuff_.Create(L"ParticleManagerIdxBuff", sizeVB);
	idxBuff_.Copy(indices_.data(), sizeIB);
	// インデックスバッファビューの作成
	ibView_.BufferLocation = idxBuff_->GetGPUVirtualAddress();
	ibView_.Format = DXGI_FORMAT_R16_UINT;
	ibView_.SizeInBytes = sizeIB;
#pragma endregion インデックスバッファ
#pragma region マテリアルバッファ
	size_t materialSize = sizeof(cMaterial);
	materialBuff_.Create(L"ParticleMaterialBuff", materialSize);
	material_ = new cMaterial();
	material_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialBuff_.Copy(material_, materialSize);
#pragma endregion
#pragma region インスタンシング生成
	for (size_t i = 0; i < kNumInstancing; i++) {
		auto device = GraphicsCore::GetInstance()->GetDevice();
		CPUParticleShaderStructs::Emitter* emitter = new CPUParticleShaderStructs::Emitter();
		CPUParticleShaderStructs::ParticleMotion* particleMotion = new CPUParticleShaderStructs::ParticleMotion();
		Instancing* instancing = new Instancing();


		// パーティクル
		instancing->particle = new Particle();
		instancing->particle->Initialize(emitter, particleMotion);

		//instancing->textureHandle = 0;

		instancing->instancingBuff.Create(L"instancing->instancingBuff", sizeof(CPUParticleShaderStructs::ParticleForGPU) * instancing->maxInstance);
		//instancing->instancingBuff.Copy(instancing->instancingDate, sizeof(ParticleForGPU) * instancing->maxInstance);

		// シェーダーリソースビュー
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		desc.Buffer.NumElements = instancing->maxInstance;
		desc.Buffer.StructureByteStride = sizeof(CPUParticleShaderStructs::ParticleForGPU);

		instancing->descriptorHandle = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(instancing->instancingBuff, &desc, instancing->descriptorHandle);
		instancing->isAlive = false;
		instancing_.emplace_back(instancing);
	}

#pragma endregion

}

void ParticleManager::Update() {
	size_t count = 0;

	for (auto it = instancing_.begin(); it != instancing_.end();) {
		if ((*it)->isAlive) {
			(*it)->particle->Update();
			(*it)->currentInstance = (*it)->particle->GetAliveParticle();
			for (size_t i = 0; i < (*it)->currentInstance; i++) {
				(*it)->instancingDate[i] = (*it)->particle->GetParticleForGPU(i);
			}
			(*it)->instancingBuff.Copy((*it)->instancingDate, sizeof((*it)->instancingDate) * (*it)->maxInstance);
			if (!(*it)->particle->GetIsAlive()) {
				(*it)->isAlive = false;
			}
			else {
				++it;
			}
		}
		else {
			break;
		}
	}
	std::sort(instancing_.begin(), instancing_.end(), &ParticleManager::CompareParticles);
}

void ParticleManager::Draw(CommandContext& commandContext, const ViewProjection& viewProjection) {
	// ルートシグネチャの設定
	commandContext.SetGraphicsRootSignature(rootSignature_);
	// パイプラインステートの設定
	commandContext.SetPipelineState(QueueType::Type::DIRECT, graphicsPipeline_);
	// プリミティブ形状を設定
	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 頂点バッファの設定
	commandContext.SetVertexBuffer(0, 1, &vbView_);
	// インデックスバッファの設定
	commandContext.SetIndexBuffer(ibView_);
	// CBVをセット（ビュープロジェクション行列）
	commandContext.SetGraphicsConstantBuffer(Parameter::RootParameter::ViewProjection, viewProjection.constBuff_.GetGPUVirtualAddress());
	// CBVをセット（Material）
	commandContext.SetGraphicsConstantBuffer(Parameter::RootParameter::Material, materialBuff_.GetGPUVirtualAddress());
	for (auto& instancing : instancing_) {
		if (instancing->isAlive) {

			// instancing用のStructuredBuffをSRVにセット
			commandContext.SetGraphicsDescriptorTable(Parameter::RootParameter::WorldTransform, instancing->descriptorHandle);
			// SRVをセット
			commandContext.SetGraphicsDescriptorTable(Parameter::RootParameter::Texture, TextureManager::GetInstance()->GetTexture(instancing->textureHandle).GetSRV());
			// サンプラー
			commandContext.SetGraphicsDescriptorTable(Parameter::RootParameter::Sampler, SamplerManager::LinearWrap);
			// 描画コマンド
			commandContext.DrawIndexedInstanced(static_cast<UINT>(indices_.size()), instancing->currentInstance, 0, 0, 0);
		}
	}
}

void ParticleManager::Shutdown() {
	for (Instancing* inst : instancing_) {
		delete inst;
	}
	// メンバー変数のリセット
	instancing_.clear();
}

void ParticleManager::AddParticle(CPUParticleShaderStructs::Emitter* emitter, CPUParticleShaderStructs::ParticleMotion* particleMotion, TextureHandle textureHandle) {
	for (auto& instancing : instancing_) {
		if (!instancing->isAlive) {
			instancing->particle->Reset();
			instancing->particle->Initialize(emitter, particleMotion);

			instancing->textureHandle = textureHandle;
			instancing->isAlive = true;
			break;
		}
	}
}
