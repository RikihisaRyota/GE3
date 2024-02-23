#pragma once
#include <d3d12.h>

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"

struct Vertex {
	Vector3 position;
	Vector2 texcoord;
};
struct Particle {
	Vector3 scale;
	Vector3 velocity;
	Vector3 rotate;
	Vector3 translate;
	uint32_t isAlive;
	uint32_t isHit;
	float aliveTime;
};
struct ParticleInfo {
	float speed;
};
struct EmitterForGPU {
	Vector3 min;
	uint32_t createParticleNum;
	Vector3 max;
	uint32_t isAlive;
	Vector3 position;
	uint32_t time;
	int32_t interval;
	uint32_t isLoop;
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

// ボールデータ
struct BallBufferData {
	Vector3 position;
	float size;
};
// ボール
struct Ball {
	Vector3 position;
	Vector3 velocity;
	float size;
	bool isAlive;
	float aliveTime;
};
// ボールカウント
struct BallCount {
	int ballCount;
};