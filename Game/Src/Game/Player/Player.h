#pragma once

#include "Engine//Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"

#include "Engine/Math/ViewProjection.h"

class CommandContext;
struct WorldTransform;
struct ViewProjection;
class Player {
public:
	Player();

	void Initialize();

	void Update();

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
private:
	void Move();
	void PlayerRotate(const Vector3& vector);

	ViewProjection* viewProjection_;

	ModelHandle playerModelHandle_;
	WorldTransform worldTransform_;
	WorldTransform animationTransform_;
};