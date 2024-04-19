#pragma once
#include <memory>
#include <list>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

#include "Engine/GPUParticleManager/GPUParticleManager.h"

class CommandContext;
class Boss {
public:
	Boss();
	void Initialize();
	void Update();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
private:

};
