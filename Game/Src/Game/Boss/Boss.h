#pragma once
#include <memory>
#include <list>

#include "Engine/Animation/Animation.h"
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
	void UpdateTransform();
	ModelHandle bossModelHandle_;
	Animation::Animation animation_;
	float animationTime_;

	WorldTransform worldTransform_;
	WorldTransform animationTransform_;
};
