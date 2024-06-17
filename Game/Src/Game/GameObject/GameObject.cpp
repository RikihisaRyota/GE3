#include "GameObject.h"

#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"

#include "Engine/ImGui/ImGuiManager.h"

GameObject::GameObject(const LevelDataLoader::GameObject& desc, const WorldTransform* worldTransform) {
	
	desc_ = desc;

	modelHandle_ = ModelManager::GetInstance()->Load(desc.fileName);
	worldTransform_.Initialize();
	worldTransform_.parent_ = worldTransform;
	worldTransform_.translate = desc_.transform.translate;
	worldTransform_.rotate = desc.transform.rotate;
	worldTransform_.scale = desc.transform.scale;
	worldTransform_.UpdateMatrix();
	if (desc.collider) {
		collider_ = new OBBCollider();
		collider_->SetName("GameObject");
		collider_->SetCenter(Transform(desc.collider->center, worldTransform_.matWorld));
		collider_->SetOrientation(worldTransform_.rotate * desc.collider->rotate);
		collider_->SetSize(desc.collider->size);
		collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
		collider_->SetCollisionAttribute(CollisionAttribute::GameObject);
		collider_->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet| CollisionAttribute::Boss);
		collider_->SetIsActive(true);
	}
}

void GameObject::Initialize(const LevelDataLoader::GameObject& desc) {
	colliderColor_ = { 0.0f,1.0f,0.0f,1.0f };
	worldTransform_.translate = desc.transform.translate;
	worldTransform_.rotate = desc.transform.rotate;
	worldTransform_.scale = desc.transform.scale;
	worldTransform_.UpdateMatrix();
}


void GameObject::Update() {
	colliderColor_ = { 0.0f,1.0f,0.0f,1.0f };
	UpdateTransform();
}

void GameObject::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection, modelHandle_, commandContext);
	collider_->DrawCollision(viewProjection, colliderColor_);
}

void GameObject::DrawImGui() {
	ImGui::PushID(this);
	if (ImGui::TreeNode(("GameObject"))) {
		ImGui::DragFloat3("Translate", &worldTransform_.translate.x);
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GameObject::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	if (collider_) {
		collider_->SetCenter(Transform(desc_.collider->center, worldTransform_.matWorld));
		collider_->SetOrientation(worldTransform_.rotate * desc_.collider->rotate);
		collider_->SetSize(desc_.collider->size);
	}
}

void GameObject::OnCollision(const ColliderDesc& collisionInfo) {
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	collisionInfo;
}
