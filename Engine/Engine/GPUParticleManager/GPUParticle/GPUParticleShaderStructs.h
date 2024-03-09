#pragma once
#include <d3d12.h>

#include "Engine/Texture/TextureHandle.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

#pragma region Utility
struct UintMinMax {
	uint32_t min;
	uint32_t max;
};

struct Vector3MinMax {
	Vector3 min;
	Vector3 max;
};

struct Vector4MinMax {
	Vector4 min;
	Vector4 max;
};

struct Vector3StartEnd {
	Vector3MinMax start;
	Vector3MinMax end;
};

struct Vector4StartEnd {
	Vector4MinMax start;
	Vector4MinMax end;
};
#pragma endregion

struct Vertex {
	Vector3 position;
	Vector2 texcoord;
};

// パーティクルの寿命
struct ParticleLifeTime {
	uint32_t time;
	uint32_t maxTime;
};

struct Particle {
	Vector3MinMax scaleRange;
	Vector3 scale;

	Vector3 rotateVelocity;
	Vector3 rotate;

	Vector3 translate;
	Vector3 velocity;
	
	Vector4 color;
	
	ParticleLifeTime particleLifeTime;
	
	uint32_t textureIndex;
	
	uint32_t isAlive;
};
// hlsli側も変更すること

// エミッターの生成範囲と生成場所
struct EmitterArea {
	Vector3MinMax area;
	Vector3 position;
};

// パーティクルのスケール
struct ScaleAnimation {
	Vector3StartEnd range;
};

// パーティクルの回転	
struct RotateAnimation {
	Vector3 rotate;
};

// パーティクルの移動
struct Velocity3D {
	Vector3 velocity;
};

// パーティクルの色
struct EmitterColor {
	Vector4StartEnd range;
};

// エミッターの生成間隔
struct EmitterFrequency {
	uint32_t time = 0;
	uint32_t interval;
	uint32_t isLoop;
	uint32_t lifeTime;
};

// パーティクルのランダム寿命
struct ParticleLifeSpan {
	UintMinMax range;
};

// エミッター
struct Emitter {
	EmitterArea emitterArea;

	ScaleAnimation scale;

	RotateAnimation rotate;

	Velocity3D velocity;

	EmitterColor color;

	EmitterFrequency frequency;

	ParticleLifeSpan particleLifeSpan;

	uint32_t textureIndex;

	uint32_t createParticleNum;

	uint32_t isAlive = true;
};

struct CreateParticle {
	uint32_t emitterIndex;
	int32_t createParticleNum;
};

struct IndirectCommand {
	struct SRV {
		D3D12_GPU_VIRTUAL_ADDRESS particleSRV;
		D3D12_GPU_VIRTUAL_ADDRESS drawIndexSRV;
	};
	SRV srv;
	D3D12_DRAW_INDEXED_ARGUMENTS drawIndex;
};