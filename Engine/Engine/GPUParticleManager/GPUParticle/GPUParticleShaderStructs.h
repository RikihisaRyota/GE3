#pragma once
#include <d3d12.h>

#include "Engine/Texture/TextureHandle.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

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
	uint32_t textureIndex;
};
// hlsli側も変更すること
struct EmitterArea {
	Vector3 min;
	Vector3 max;
	Vector3 position;
};

struct ScaleAnimation {

};

struct RotateAnimation {

};

struct Velocity3D {

};

struct EmitterColor {
	Vector4 min;
	Vector4 max;
};

struct EmitterFrequency {
	uint32_t time;
	int32_t interval;
	uint32_t isLoop;
};

struct Emitter {
	EmitterArea area;

	EmitterColor color;
	
	EmitterFrequency frequency;

	uint32_t createParticleNum;

	TextureHandle textureHandle;
	
	uint32_t isAlive = true;
};

struct EmitterForGPU {
	EmitterArea area;

	EmitterFrequency frequency;

	uint32_t createParticleNum;

	uint32_t textureIndex;

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