#pragma once
/**
 * @file Skinning.h
 * @brief Skinningの管理
 */
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
#include "Engine/Graphics/DefaultBuffer.h"
#include "Engine/Graphics/CommandContext.h"

namespace Engine {
	namespace Animation {
		extern PipelineState pipelineState;
		extern RootSignature rootSignature;

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
			// スキンクラスター生成
			void CreateSkinCluster(const Skeleton& skeleton, const ModelHandle& modelHandle);
			// アップデート
			void Update(const Skeleton& skeleton, CommandContext& commandContext, const ModelHandle& modelHandle);

			std::vector<Matrix4x4> inverseBindPoseMatrices;

			UploadBuffer influenceResource;
			DescriptorHandle influenceHandle;
			std::span<VertexInfluence> mappedInfluence;

			UploadBuffer paletteResource;
			std::span<WellForGPU> mappedPalette;
			DescriptorHandle paletteHandle;

			DefaultBuffer vertexBuffer;
			uint32_t vertexBufferDescriptorIndex;
			DescriptorHandle outputVertexBufferView;
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

			UploadBuffer skinningInfomation;
		};
	}
}