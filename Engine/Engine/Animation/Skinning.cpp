#include "Skinning.h"

#include <assert.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"

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

	auto verticesSize = model.GetMeshData().at(0)->vertices.size();
	influenceResource.Create(L"skinClusterInfluenceResource", sizeof(VertexInfluence) * verticesSize);
	
	VertexInfluence* mappedInfluence = new VertexInfluence[verticesSize];
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * verticesSize);
	this->mappedInfluence = { mappedInfluence, verticesSize };


	influenceBufferView.BufferLocation = influenceResource.GetGPUVirtualAddress();
	influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * verticesSize);
	influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	inverseBindPoseMatrices.resize(jointSize);
	std::generate(inverseBindPoseMatrices.begin(), inverseBindPoseMatrices.end(), MakeIdentity4x4);

	for (const auto& jointWeight : model.GetMeshData().at(0)->skinClusterData) {
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
	influenceResource.Copy(this->mappedInfluence.data(), sizeof(VertexInfluence) * verticesSize);
}

void SkinCluster::Update(const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < inverseBindPoseMatrices.size());

		mappedPalette[jointIndex].skeletonSpaceMatrix =
			inverseBindPoseMatrices[jointIndex] * skeleton.joints.at(jointIndex).skeletonSpaceMatrix;
		mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Transpose(Inverse(mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
	paletteResource.Copy(mappedPalette.data(), sizeof(WellForGPU) * skeleton.joints.size());
}
