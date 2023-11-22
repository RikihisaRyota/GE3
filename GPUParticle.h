#pragma once

#include "Game/Engine/Graphics/PipelineState.h"
#include "Game/Engine/Graphics/RootSignature.h"

class GPUParticle {
public:
	void Initialize();
	void Update();
	void Render();
private:
	ComputePipeline updatePipeline_;
	RootSignature rootSignature_;

};

