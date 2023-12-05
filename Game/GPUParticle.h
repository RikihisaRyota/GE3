#pragma once

#include <memory>
#include <vector>

#include <wrl.h>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/GpuResource.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"

class GPUParticle {
public:
	struct UpdateParticle {
		Vector3 position;
		float speed;
	};
public:
	void Initialize();
	void Update(CommandContext& commandContext);
	void Render(CommandContext& commandContext);

private:
	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<PipelineState> computePipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> computeRootSignature_;

	GpuResource rwStructuredBuffer_;
	// UAVハンドル
	DescriptorHandle uavHandle_;
	
	// Update用
	uint32_t* updateParticle_;
	const uint32_t kNumThread = 256;
};