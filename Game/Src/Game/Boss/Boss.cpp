#include "Boss.h"

#include <numbers>

#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"

Boss::Boss() {
	bossModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/boss.gltf");
	animation_.Initialize(bossModelHandle_);
	twoHandedAttackHandle_ = animation_.GetAnimationHandle("twoHandedAttack");
#pragma region コライダー
	collider_ = new OBBCollider();
	collider_->SetName("Boss");
	auto& mesh = ModelManager::GetInstance()->GetModel(bossModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Boss);
	collider_->SetCollisionMask(~CollisionAttribute::Boss);
	collider_->SetIsActive(true);
#pragma endregion
	worldTransform_.Initialize();
	animationTransform_.Initialize();
}

void Boss::Initialize() {
	worldTransform_.Reset();
	worldTransform_.translate.z = 5.0f;
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	UpdateTransform();
}

void Boss::Update(CommandContext& commandContext) {
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
					   .min = {0.005f,0.005f,0.005f},
					   .max = {0.005f,0.005f,0.005f},
				   },
				   .end{
					   .min = {0.001f,0.001f,0.001f},
					   .max = {0.001f,0.001f,0.001f},
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

	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	static const float kCycle = 360.0f;
	animationTime_ += 1.0f;
	animationTime_ = std::fmodf(animationTime_, kCycle);
	animation_.Update(twoHandedAttackHandle_,animationTime_ / kCycle,commandContext,bossModelHandle_);
	UpdateTransform();
}

void Boss::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, viewProjection, bossModelHandle_, commandContext);
	//animation_.DrawBox(animationTransform_,viewProjection);
	animation_.DrawLine(animationTransform_);
	collider_->DrawCollision(viewProjection, colliderColor_);
}

void Boss::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	auto& mesh = ModelManager::GetInstance()->GetModel(bossModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	animationTransform_.UpdateMatrix();
}

void Boss::OnCollision(const ColliderDesc& desc) {
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	desc;
}
