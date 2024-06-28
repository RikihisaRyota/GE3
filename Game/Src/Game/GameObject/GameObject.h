#pragma once

#include <memory>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Collision/Collider.h"
#include "Engine/LevelDataLoader/LevelDataLoader.h"

struct ViewProjection;
class CommandContext;
class GameObject {
public:
	GameObject(const LevelDataLoader::GameObject& desc,const WorldTransform* worldTransform = nullptr);
	void Initialize(const LevelDataLoader::GameObject& desc);

	void Update();

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawDebug(const ViewProjection& viewProjection);

	void DrawImGui();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

private:
	void UpdateTransform();
	void OnCollision(const ColliderDesc& collisionInfo);
	WorldTransform worldTransform_;
	OBBCollider* collider_;
	ModelHandle modelHandle_;

	LevelDataLoader::GameObject desc_;
	Vector4 colliderColor_;
};