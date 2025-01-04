#pragma once
/**
 * @file GameObject.h
 * @brief GameObject
 */
#include <memory>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Collision/Collider.h"
#include "Engine/LevelDataLoader/LevelDataLoader.h"

struct ViewProjection;
class CommandContext;
class GameObject {
public:
	// コンストラクタ
	GameObject(const LevelDataLoader::GameObject& desc,const WorldTransform* worldTransform = nullptr);
	// 初期化
	void Initialize(const LevelDataLoader::GameObject& desc);
	// 更新
	void Update();
	// 描画
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	// Debug
	void DrawDebug();
	void DrawImGui();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	// 外部から消すことあり
	OBBCollider* GetCollider() { return collider_; }
	const std::string& GetObjectName() const { return desc_.objectName.value(); }
private:
	// UpdateTransform
	void UpdateTransform();
	// 当たり判定
	void OnCollision(const ColliderDesc& collisionInfo);
	WorldTransform worldTransform_;
	OBBCollider* collider_;
	ModelHandle modelHandle_;

	LevelDataLoader::GameObject desc_;
	Vector4 colliderColor_;
};