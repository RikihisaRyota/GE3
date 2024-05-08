#include "PlayerBullet.h"

#include <numbers>

#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Texture/TextureManager.h"

void PlayerBullet::Create(GPUParticleManager* GPUParticleManager, const Vector3& position, const Vector3& velocity, uint32_t time) {
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball");
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	gpuParticleManager_ = GPUParticleManager;
	
	worldTransform_.Initialize();
	worldTransform_.translate = position;
	
	for (auto& sub : secondBullet_) {
		sub.Initialize();
		sub.parent_ = &worldTransform_;
	}
	worldTransform_.UpdateMatrix();     
	velocity_ = velocity;
	time_ = time;
	isAlive_ = true;
}

void PlayerBullet::Update() {
	time_--;
	if (time_ <= 0) {
		isAlive_ = false;
	}
	worldTransform_.translate += velocity_;
	//worldTransform_.rotate.z += 0.1f;
	worldTransform_.UpdateMatrix();
	for (int i = 0; auto & sub : secondBullet_) {
		float angle = 2.0f * std::numbers::pi_v<float> * i / kNumSubBullet;
		float radius = 2.0f; // 衛星の半径

		// サブオブジェクトの相対的な位置を計算
		Vector3 relativePosition = {
			radius * std::cosf(angle),
			radius * std::sinf(angle),
			0.0f
		};

		// サブオブジェクトの位置を更新
		sub.translate = MakeRotateXYZMatrix(Normalize(velocity_)) * relativePosition;
		sub.UpdateMatrix();
		i++;

		GPUParticleShaderStructs::Emitter emitterForGPU = {
		.emitterArea{
				.area{
					.min = {-0.1f,-0.1f,-0.1f},
					.max = {0.1f,0.1f,0.1f},
				},
				.position = {MakeTranslateMatrix(sub.matWorld)},
			},

		.scale{
			.range{
				.start{
					.min = {0.01f,0.01f,0.01f},
					.max = {0.01f,0.01f,0.01f},
				},
				.end{
					.min = {0.01f,0.01f,0.01f},
					.max = {0.01f,0.01f,0.01f},
				},
			},
		},

		.rotate{
			.rotate = {0.0f,0.0f,0.3f},
		},

		.velocity{
			.range{
				.min = {-0.1f,-0.1f,-0.1f},
				.max = {0.1f,0.1f,0.1f},
			}
		},

		.color{
			.range{
				.start{
					.min = {0.2f,0.2f,0.1f,1.0f},
					.max = {0.6f,0.4f,0.2f,1.0f},
				},
				.end{
					.min = {0.8f,0.1f,0.1f,0.1f},
					.max = {0.9f,0.15f,0.1f,0.1f},
				},
			},
		},

		.frequency{
			.interval = 0,
			.isLoop = false,
			.lifeTime = 0,
		},

		.particleLifeSpan{
			.range {
				.min = 5,
				.max = 10,
			}
		},

		.textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

		.createParticleNum = 1 << 10,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU);
	}
	//// 弾本体
	//// 0
	//{
	//	GPUParticleShaderStructs::Emitter emitterForGPU = {
	//	.emitterArea{
	//			.area{
	//				.min = {-0.5f,-0.5f,-0.5f},
	//				.max = {0.5f,0.5f,0.5f},
	//			},
	//			.position = {MakeTranslateMatrix(worldTransform_.matWorld)},
	//		},

	//	.scale{
	//		.range{
	//			.start{
	//				.min = {0.5f,0.5f,0.5f},
	//				.max = {0.5f,0.5f,0.5f},
	//			},
	//			.end{
	//				.min = {0.1f,0.1f,0.1f},
	//				.max = {0.1f,0.1f,0.1f},
	//			},
	//		},
	//	},

	//	.rotate{
	//		.rotate = {0.0f,0.0f,0.0f},
	//	},

	//	.velocity{
	//		.range{
	//			.min = {-0.3f,-0.3f,-0.3f},
	//			.max = {0.3f,0.3f,0.3f},
	//		}
	//	},

	//	.color{
	//		.range{
	//			.start{
	//				.min = {0.5f,0.2f,0.1f,1.0f},
	//				.max = {0.8f,0.4f,0.2f,1.0f},
	//			},
	//			.end{
	//				.min = {0.8f,0.1f,0.1f,0.1f},
	//				.max = {0.9f,0.15f,0.1f,0.1f},
	//			},
	//		},
	//	},

	//	.frequency{
	//		.interval = 0,
	//		.isLoop = false,
	//		.lifeTime = 0,
	//	},

	//	.particleLifeSpan{
	//		.range{
	//			.min = 10,
	//			.max = 15,
	//		}
	//	},

	//	.textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

	//	.createParticleNum = 1 << 5,
	//	};
	//	gpuParticleManager_->CreateParticle(emitterForGPU);
	//}
}

void PlayerBullet::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//ModelManager::GetInstance()->Draw(worldTransform_, viewProjection, modelHandle_, commandContext);
}
