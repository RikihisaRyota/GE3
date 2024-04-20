#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <d3d12.h>

#include "Engine/Animation/Skeleton.h"
#include "Engine/Graphics/DescriptorHandle.h"
#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Math/Matrix4x4.h"
#include "Engine/Model/ModelHandle.h"

static const uint32_t kNumMaxInfluence = 4;

struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;
};
struct SkinCluster {
	std::vector<Matrix4x4> inverseBindPoseMatrices;

	UploadBuffer influenceResource;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
	std::span<VertexInfluence> mappedInfluence;

	UploadBuffer paletteResource;
	std::span<WellForGPU> mappedPalette;
	DescriptorHandle paletteHandle;

	void Update(const Skeleton& skeleton);
	void CreateSkinCluster(const Skeleton& skeleton,const ModelHandle& modelHandle);
};