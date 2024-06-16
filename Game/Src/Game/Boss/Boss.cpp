#include "Boss.h"

#include <numbers>

#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/ImGui/ImGuiManager.h"

#include "Engine/Json/JsonUtils.h"

Boss::Boss() {
	bossModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/boss.gltf");
	animation_.Initialize("Resources/Animation/Boss/animation.gltf", bossModelHandle_);


	bossStateManager_ = std::make_unique<BossStateManager>();
	bossStateManager_->SetBoss(this);
	worldTransform_.Initialize();
	animationTransform_.Initialize();

	JSON_OPEN("Resources/Data/Boss/boss.json");
	JSON_OBJECT("bossCollision");
	JSON_LOAD(colliderSize_);
	JSON_ROOT();
	JSON_OBJECT("bossProperties");
	JSON_LOAD(offset_);
	JSON_LOAD(animationWorldTransformOffset_);
	JSON_ROOT();
	JSON_CLOSE();

#pragma region コライダー
	for (auto& joint : animation_.skeleton.joints) {
		if (!joint.parent) {
			continue;
		}
		bossCollider_[joint.name] =new OBBCollider();
		bossCollider_[joint.name]->SetName("Boss");
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * animationTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * animationTransform_.matWorld;

		Vector3 born = (MakeTranslateMatrix(worldMatrix) - MakeTranslateMatrix(parentMatrix));
		bossCollider_[joint.name]->SetCenter(MakeTranslateMatrix(parentMatrix) + born * 0.5f);
		bossCollider_[joint.name]->SetOrientation(MakeLookRotation(born.Normalized()));
		bossCollider_[joint.name]->SetSize({ born.Length() * colliderSize_, born.Length() * colliderSize_,born.Length() });
		bossCollider_[joint.name]->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
		bossCollider_[joint.name]->SetCollisionAttribute(CollisionAttribute::Boss);
		bossCollider_[joint.name]->SetCollisionMask(CollisionAttribute::Player| CollisionAttribute::PlayerBullet);
		bossCollider_[joint.name]->SetIsActive(true);
	}
#pragma endregion

}

void Boss::Initialize() {
	bossStateManager_->Initialize();

	worldTransform_.Reset();
	worldTransform_.translate = offset_;
	worldTransform_.rotate = MakeRotateYAngleQuaternion(DegToRad(180.0f));
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	animationTransform_.translate = animationWorldTransformOffset_;
	UpdateTransform();
	UpdateCollider();
}

void Boss::Update(CommandContext& commandContext) {
	UpdateGPUParticle();
	bossStateManager_->Update(commandContext);
	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	UpdateTransform();
	UpdateCollider();
}

void Boss::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, viewProjection, bossModelHandle_, commandContext);
}

void Boss::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		auto& color = ModelManager::GetInstance()->GetModel(bossModelHandle_).GetMaterialColor();
		ImGui::DragFloat3("color", &color.x, 0.1f, 0.0f, 1.0f);
		ModelManager::GetInstance()->GetModel(bossModelHandle_).SetMaterialColor(color);
		ImGui::DragFloat3("translate", &worldTransform_.translate.x, 0.1f);
		ImGui::DragFloat3("animationTranslate", &animationTransform_.translate.x, 0.1f);
		ImGui::Text("matWorld: x:%f,y:%f,z:%f", MakeTranslateMatrix(worldTransform_.matWorld).x, MakeTranslateMatrix(worldTransform_.matWorld).y, MakeTranslateMatrix(worldTransform_.matWorld).z, 0.1f);
		if (ImGui::TreeNode("Properties")) {
			ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
			ImGui::DragFloat3("AnimationWorldTransformOffset_", &animationWorldTransformOffset_.x, 0.1f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/boss.json");
				JSON_OBJECT("bossProperties");
				JSON_SAVE(offset_);
				JSON_SAVE(animationWorldTransformOffset_);
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Collision")) {
			ImGui::DragFloat("colliderSize_", &colliderSize_, 0.1f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/boss.json");
				JSON_OBJECT("bossCollision");
				JSON_SAVE(colliderSize_);
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	bossStateManager_->DrawImGui();

}

void Boss::DrawDebug(const ViewProjection& viewProjection) {
	
	for (auto& collider : bossCollider_) {
		collider.second->DrawCollision(viewProjection, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	}
	animation_.DrawLine(animationTransform_);
}

void Boss::UpdateCollider() {

	for (auto& joint : animation_.skeleton.joints) {
		if (!joint.parent) {
			continue;
		}
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * animationTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * animationTransform_.matWorld;

		Vector3 born = (MakeTranslateMatrix(worldMatrix) - MakeTranslateMatrix(parentMatrix));
		bossCollider_[joint.name]->SetCenter(MakeTranslateMatrix(parentMatrix) + born * 0.5f);
		bossCollider_[joint.name]->SetOrientation(MakeLookRotation(born.Normalized()));
		bossCollider_[joint.name]->SetSize({ born.Length() * colliderSize_, born.Length() * colliderSize_,born.Length() });
	}
}

void Boss::UpdateGPUParticle() {
	for (auto& joint : animation_.skeleton.joints) {
		if (!joint.parent.has_value()) {
			continue;
		}
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * worldTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * worldTransform_.matWorld;

		Vector3 worldPos = MakeTranslateMatrix(worldMatrix);
		Vector3 parentPos = MakeTranslateMatrix(parentMatrix);
		Vector3 born = (worldPos - parentPos);
		// 0
		{
			GPUParticleShaderStructs::Emitter emitterForGPU = {
		   .emitterArea{
				   .sphere{
						.radius = born.Length() * 0.5f,
					},
					.position{worldPos},
					.type = 1,
			   },

		   .scale{
			   .range{
				   .start{
					   .min = {0.1f,0.1f,0.1f},
					   .max = {0.1f,0.1f,0.1f},
				   },
				   .end{
					   .min = {0.01f,0.01f,0.01f},
					   .max = {0.01f,0.01f,0.01f},
				   },
			   },
		   },

		   .rotate{
			   .rotate = 0.0f,
		   },

		   .velocity{
			   .range{
				   .min = {0.0f,0.0f,0.0f},
				   .max = {0.0f,0.0f,0.0f},
			   }
		   },

		   .color{
			   .range{
				   .start{
					   .min = {0.0f,1.0f,0.0f,1.0f},
					   .max = {0.0f,1.0f,0.0f,1.0f},
				   },
				   .end{
					   .min = {0.0f,0.2f,0.0f,1.0f},
					   .max = {0.0f,0.2f,0.0f,1.0f},
				   },
			   },
		   },

		   .frequency{
			   .interval = 1,
			   .isLoop = false,
			   //.lifeTime = 120,
		   },

		   .particleLifeSpan{
			   .range{
				   .min = 1,
				   .max = 2,
			   }
		   },

		   .textureIndex = 0,

		   .createParticleNum = 1 << 10,
			};

			gpuParticleManager_->CreateParticle(emitterForGPU);
		}
	}
}

void Boss::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	animationTransform_.UpdateMatrix();
}

void Boss::OnCollision(const ColliderDesc& desc) {
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	desc;
}
