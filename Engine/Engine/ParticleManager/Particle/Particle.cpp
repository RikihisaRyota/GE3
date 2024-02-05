#include "Particle.h"

#include "Engine/Math/MyMath.h"

void Particle::Initialize(Emitter* emitter, ParticleMotion* particleMotion) {
	emitter_ = emitter;
	originalParticle_ = particleMotion;
	flameInterval_ = emitter->flameInterval;
	emitter->flameInterval = 0;
	isAlive_ = true;
}

void Particle::Update() {
	// 生成
	if (emitter_->aliveTime > 0) {
		if (emitter_->flameInterval <= 0) {
			for (size_t i = 0; i < emitter_->inOnce; i++) {
				// パーティクル
				ParticleWorldTransform* particle = new ParticleWorldTransform();
				particle->motion = *originalParticle_;
				float angle = 0.0f;
				if (emitter_->angle.start != emitter_->angle.end) {
					angle = rnd_.NextFloatRange(emitter_->angle.start, emitter_->angle.end);
				}
				else if (emitter_->angle.current != 0.0f) {
					angle = emitter_->angle.current;
				}
				Vector3 vector = { std::cos(angle),std::sin(angle),0.0f };
				particle->motion.velocity.velocity = vector * rnd_.NextFloatRange(particle->motion.velocity.speed - particle->motion.velocity.randomRange, particle->motion.velocity.speed + particle->motion.velocity.randomRange);
				particle->motion.position.x = rnd_.NextFloatRange(emitter_->spawn.position.x - emitter_->spawn.rangeX, emitter_->spawn.position.x + emitter_->spawn.rangeX);
				particle->motion.position.y = rnd_.NextFloatRange(emitter_->spawn.position.y - emitter_->spawn.rangeY, emitter_->spawn.position.y + emitter_->spawn.rangeY);

				particle->motion.aliveTime.time = rnd_.NextIntRange(particle->motion.aliveTime.time - particle->motion.aliveTime.randomRange, particle->motion.aliveTime.time + particle->motion.aliveTime.randomRange);
				particle->motion.aliveTime.maxTime = particle->motion.aliveTime.time;

				particle->motion.scale.startScale.x = rnd_.NextFloatRange(emitter_->scale.startScale.x - emitter_->randomScale.startRandomRange.x, emitter_->scale.startScale.x + emitter_->randomScale.startRandomRange.x);
				particle->motion.scale.startScale.y = rnd_.NextFloatRange(emitter_->scale.startScale.y - emitter_->randomScale.startRandomRange.y, emitter_->scale.startScale.y + emitter_->randomScale.startRandomRange.y);
				particle->motion.scale.startScale.z = rnd_.NextFloatRange(emitter_->scale.startScale.z - emitter_->randomScale.startRandomRange.z, emitter_->scale.startScale.z + emitter_->randomScale.startRandomRange.z);

				particle->motion.scale.interimScale.x = rnd_.NextFloatRange(emitter_->scale.interimScale.x - emitter_->randomScale.interimRandomRange.x, emitter_->scale.interimScale.x + emitter_->randomScale.interimRandomRange.x);
				particle->motion.scale.interimScale.y = rnd_.NextFloatRange(emitter_->scale.interimScale.y - emitter_->randomScale.interimRandomRange.y, emitter_->scale.interimScale.y + emitter_->randomScale.interimRandomRange.y);
				particle->motion.scale.interimScale.z = rnd_.NextFloatRange(emitter_->scale.interimScale.z - emitter_->randomScale.interimRandomRange.z, emitter_->scale.interimScale.z + emitter_->randomScale.interimRandomRange.z);

				particle->motion.scale.endScale.x = rnd_.NextFloatRange(emitter_->scale.endScale.x - emitter_->randomScale.endRandomRange.x, emitter_->scale.endScale.x + emitter_->randomScale.endRandomRange.x);
				particle->motion.scale.endScale.y = rnd_.NextFloatRange(emitter_->scale.endScale.y - emitter_->randomScale.endRandomRange.y, emitter_->scale.endScale.y + emitter_->randomScale.endRandomRange.y);
				particle->motion.scale.endScale.z = rnd_.NextFloatRange(emitter_->scale.endScale.z - emitter_->randomScale.endRandomRange.z, emitter_->scale.endScale.z + emitter_->randomScale.endRandomRange.z);

				particle->motion.scale.currentScale = particle->motion.scale.startScale;

				particle->motion.isAlive = true;
				// ワールドトランスフォーム
				particle->constantDate.color = particle->motion.color.currentColor;
				particle->scale = particle->motion.scale.currentScale;
				particle->rotate = particle->motion.rotate.currentRotate;
				particle->transform = particle->motion.position;
				particle->UpdateMatrix();
				particleWorldTransform_.emplace_back(particle);

			}
			emitter_->flameInterval = flameInterval_;
		}
		emitter_->flameInterval--;
		emitter_->aliveTime--;
	}
	else {
		emitter_->isAlive = false;
	}
	numAliveParticle_ = 0;
	// 更新
	for (auto& particle : particleWorldTransform_) {
		// 生きてるかどうか
		if (particle->motion.isAlive) {
			if (particle->motion.aliveTime.time <= 0) {
				particle->motion.isAlive = false;
				isAlive_ = false;
			}
			else {
				float t = 1.0f - float(particle->motion.aliveTime.time) / float(particle->motion.aliveTime.maxTime);
				// 更新
				// 色
				particle->motion.color.currentColor = Lerp(particle->motion.color.startColor, particle->motion.color.endColor, std::clamp(t, 0.0f, 1.0f));
				// スケール
				if (particle->motion.scale.interimScale == Vector3(0.0f, 0.0f, 0.0f)) {
					particle->motion.scale.currentScale = Lerp(particle->motion.scale.startScale, particle->motion.scale.endScale, std::clamp(t, 0.0f, 1.0f));
				}
				else {
					if (t <= 0.5f) {
						particle->motion.scale.currentScale = Lerp(particle->motion.scale.startScale, particle->motion.scale.interimScale, std::clamp(t, 0.0f, 0.5f));
					}
					else {
						particle->motion.scale.currentScale = Lerp(particle->motion.scale.interimScale, particle->motion.scale.endScale, std::clamp(t, 0.5f, 1.0f));
					}
				}
				// 回転
				particle->motion.rotate.currentRotate += particle->motion.rotate.addRotate;
				// 移動
				particle->motion.position += particle->motion.velocity.velocity;
				particle->motion.velocity.velocity += particle->motion.acceleration_;

				particle->scale = particle->motion.scale.currentScale;
				particle->rotate = particle->motion.rotate.currentRotate;
				particle->transform = particle->motion.position;
				particle->UpdateMatrix();
				particle->constantDate.color = particle->motion.color.currentColor;
				particle->motion.aliveTime.time--;
				numAliveParticle_++;
			}
		}
		else {
			break;
		}
	}
	// 死んでいたら消す
	particleWorldTransform_.erase(std::remove_if(
		particleWorldTransform_.begin(), particleWorldTransform_.end(),
		[](const ParticleWorldTransform* particle) {
			return !particle->motion.isAlive;
		}),
		particleWorldTransform_.end()
	);
	if (particleWorldTransform_.empty()) {
		isAlive_ = false;
	}
	else {
		isAlive_ = true;
	}

}

void Particle::Reset() {
	particleWorldTransform_.clear();
	isAlive_ = false;
	numAliveParticle_ = 0;
	emitter_ = nullptr;
	originalParticle_ = nullptr;
}


void Particle::ParticleWorldTransform::UpdateMatrix() {
	Matrix4x4 matScale, matRot, matTrans;
	matScale = MakeIdentity4x4();
	matRot = MakeIdentity4x4();
	matTrans = MakeIdentity4x4();

	// スケール、回転、平行移動行列の計算
	matScale = MakeScaleMatrix(scale);
	matRot *= MakeRotateXYZMatrix(rotate);
	matTrans = MakeTranslateMatrix(transform);

	// ワールド行列の合成
	constantDate.matWorld = MakeIdentity4x4(); // 変形をリセット
	constantDate.matWorld *= matScale;          // ワールド行列にスケーリングを反映
	constantDate.matWorld *= matRot;            // ワールド行列に回転を反映
	constantDate.matWorld *= matTrans;          // ワールド行列に平行移動を反映
}
