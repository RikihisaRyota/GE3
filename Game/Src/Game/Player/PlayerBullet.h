#pragma once

#include <array>

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Texture/TextureHandle.h"

class CommandContext;
class PlayerBullet {
public:
	void Create(GPUParticleManager* GPUParticleManager,const Vector3& position,const Vector3& velocity,uint32_t time);
	void Update();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	bool GetIsAlive() { return isAlive_; }
private:
	static const uint32_t kNumSubBullet = 5;
	GPUParticleManager* gpuParticleManager_;

	ModelHandle modelHandle_;
	TextureHandle gpuTexture_;
	WorldTransform worldTransform_;
	std::array<WorldTransform,kNumSubBullet> secondBullet_;
	Vector3 velocity_;
	bool isAlive_;
	uint32_t time_;
};