#pragma once
#include "../ParticleShaderStruct.h"

#include <memory>
#include <vector>

#include "Engine/Math/Random.h"
#include "Engine/Math/Vector4.h"

class Particle {
	struct ParticleWorldTransform {
		CPUParticleShaderStructs::ParticleMotion motion;
		Vector3 scale;
		Vector3 rotate;
		Vector3 transform;
		CPUParticleShaderStructs::ParticleForGPU constantDate;
		void UpdateMatrix();
	};
public:
	void Initialize(CPUParticleShaderStructs::Emitter* emitter, CPUParticleShaderStructs::ParticleMotion* particleMotion);
	void Update();
	void Reset();

	CPUParticleShaderStructs::ParticleForGPU GetParticleForGPU(size_t num) { return particleWorldTransform_.at(num)->constantDate; }
	uint32_t GetAliveParticle() { return numAliveParticle_; }
	bool GetIsAlive() { return isAlive_; }
private:
	static bool CompareParticles(const ParticleWorldTransform* a, const ParticleWorldTransform* b) {
		return a->motion.isAlive > b->motion.isAlive;
	}
	Random::RandomNumberGenerator rnd_;
	bool isAlive_;
	uint32_t numAliveParticle_;
	int32_t flameInterval_;
	CPUParticleShaderStructs::ParticleMotion* originalParticle_;
	CPUParticleShaderStructs::Emitter* emitter_;
	std::vector<ParticleWorldTransform*> particleWorldTransform_;
};