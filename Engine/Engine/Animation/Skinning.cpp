#include "Skinning.h"

#include <assert.h>
#include <d3dx12.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"

namespace Animation {
	enum {
		kWell,
		kInputVertex,
		kInfluence,
		kOutputVertex,
		kSkinningInfomation,
		kCount,
	};

	PipelineState pipelineState;
	RootSignature rootSignature;

	// 初期化関数の定義
	void Initialize() {
		{
			CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};
			rootParameters[kWell].InitAsShaderResourceView(0);
			rootParameters[kInputVertex].InitAsShaderResourceView(1);
			rootParameters[kInfluence].InitAsShaderResourceView(2);
			rootParameters[kOutputVertex].InitAsUnorderedAccessView(0);
			rootParameters[kSkinningInfomation].InitAsConstantBufferView(0);

			D3D12_ROOT_SIGNATURE_DESC desc{};
			desc.pParameters = rootParameters;
			desc.NumParameters = _countof(rootParameters);
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			rootSignature.Create(L"SkinningRootSigunature", desc);
		}
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
			auto cs = ShaderCompiler::Compile(L"Resources/Shaders/Skinning/Skinning.hlsl", L"cs_6_0");
			desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
			desc.pRootSignature = rootSignature;

			pipelineState.Create(L"SkinningPipelineState", desc);
		}
	}

	void Release() {
		pipelineState.Release();
		rootSignature.Release();
	}

	void SkinCluster::CreateSkinCluster(const Skeleton& skeleton, const ModelHandle& modelHandle) {
		auto graphics = GraphicsCore::GetInstance();
		auto device = graphics->GetDevice();

		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

		auto jointSize = skeleton.joints.size();
		paletteResource.Create(L"skinClusterPaletteResource", sizeof(WellForGPU) * jointSize);

		WellForGPU* mappedPalette = new WellForGPU[jointSize];
		std::memset(mappedPalette, 0, sizeof(WellForGPU) * jointSize);
		this->mappedPalette = { mappedPalette , jointSize };
		paletteHandle = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
		paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
		paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		paletteSrvDesc.Buffer.FirstElement = 0;
		paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		paletteSrvDesc.Buffer.NumElements = UINT(jointSize);
		paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);

		device->CreateShaderResourceView(paletteResource, &paletteSrvDesc, paletteHandle);

		auto verticesSize = UINT(model.GetAllVertexCount());
		influenceResource.Create(L"skinClusterInfluenceResource", sizeof(VertexInfluence) * verticesSize);

		VertexInfluence* mappedInfluence = new VertexInfluence[verticesSize];
		std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * verticesSize);
		this->mappedInfluence = { mappedInfluence, verticesSize };

		influenceHandle = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC influenceSrvDesc{};
		influenceSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
		influenceSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		influenceSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		influenceSrvDesc.Buffer.FirstElement = 0;
		influenceSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		influenceSrvDesc.Buffer.NumElements = verticesSize;
		influenceSrvDesc.Buffer.StructureByteStride = sizeof(VertexInfluence);
		device->CreateShaderResourceView(influenceResource, &influenceSrvDesc, influenceHandle);

		inverseBindPoseMatrices.resize(jointSize);
		std::generate(inverseBindPoseMatrices.begin(), inverseBindPoseMatrices.end(), MakeIdentity4x4);


		auto vertexBufferSize = model.GetVertexBuffer().GetBufferSize();

		vertexBuffer.Create(L"SkinClusterVertexBuffer", vertexBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		vertexBufferView.BufferLocation = vertexBuffer.GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = UINT(vertexBufferSize);
		vertexBufferView.StrideInBytes = sizeof(Model::Vertex);

		outputVertexBufferView = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		vertexBufferDescriptorIndex = GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetFreeDescriptors();

		D3D12_UNORDERED_ACCESS_VIEW_DESC vertexUAVDesc{};
		vertexUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		vertexUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		vertexUAVDesc.Buffer.FirstElement = 0;
		vertexUAVDesc.Buffer.NumElements = verticesSize;
		vertexUAVDesc.Buffer.StructureByteStride = sizeof(Model::Vertex);
		vertexUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		device->CreateUnorderedAccessView(
			vertexBuffer,
			nullptr,
			&vertexUAVDesc,
			outputVertexBufferView);

		for (auto& mesh : model.GetMeshData()) {
			for (const auto& jointWeight : mesh->skinClusterData) {
				auto it = skeleton.jointMap.find(jointWeight.first);
				if (it == skeleton.jointMap.end()) {
					continue;
				}
				inverseBindPoseMatrices.at((*it).second) = jointWeight.second.inverseBindPoseMatrix;
				for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
					auto& currentInfluence = this->mappedInfluence[vertexWeight.vertexIndex];
					for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
						if (currentInfluence.weights.at(index) == 0.0f) {
							currentInfluence.weights.at(index) = vertexWeight.weight;
							currentInfluence.jointIndices.at(index) = (*it).second;
							break;
						}
					}
				}
			}
		}
		influenceResource.Copy(this->mappedInfluence.data(), sizeof(VertexInfluence) * verticesSize);

		skinningInfomation.Create(L"SkinningInfomation", sizeof(UINT));
		skinningInfomation.Copy(&verticesSize, sizeof(UINT));
	}

	void SkinCluster::Update(const Skeleton& skeleton, CommandContext& commandContext, const ModelHandle& modelHandle) {
		for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
			assert(jointIndex < inverseBindPoseMatrices.size());

			mappedPalette[jointIndex].skeletonSpaceMatrix =
				inverseBindPoseMatrices[jointIndex] * skeleton.joints.at(jointIndex).skeletonSpaceMatrix;
			mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
				Transpose(Inverse(mappedPalette[jointIndex].skeletonSpaceMatrix));
		}
		paletteResource.Copy(mappedPalette.data(), sizeof(WellForGPU) * skeleton.joints.size());

		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		//commandContext.BeginEvent(QueueType::Type::COMPUTE,L"Skinning");
		commandContext.SetComputeRootSignature(rootSignature);
		commandContext.SetPipelineState(QueueType::Type::COMPUTE,pipelineState);

		commandContext.TransitionResource(QueueType::Type::COMPUTE,paletteResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE,model.GetVertexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE,influenceResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE,vertexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE,skinningInfomation, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.SetComputeShaderResource(kWell, paletteResource.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(kInputVertex, model.GetVertexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(kInfluence, influenceResource.GetGPUVirtualAddress());
		commandContext.SetComputeUAV(kOutputVertex, vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(kSkinningInfomation, skinningInfomation.GetGPUVirtualAddress());
		commandContext.Dispatch(UINT(model.GetAllVertexCount() + 1023) / 1024, 1, 1);

		commandContext.UAVBarrier(QueueType::Type::COMPUTE,vertexBuffer);
		//commandContext.EndEvent(QueueType::Type::COMPUTE);
	}

}