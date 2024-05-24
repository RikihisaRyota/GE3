#pragma once
#include <d3d12.h>

#include "Engine/Texture/TextureHandle.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace GPUParticleShaderStructs {
	// hlsli側も変更するように
	static const UINT ComputeThreadBlockSize = 8;
	static const UINT MaxParticleShouldBeSquare = 20;
	static const UINT MaxParticleNum = 1 << MaxParticleShouldBeSquare;
	static const UINT MaxEmitterNum = 100;


#pragma region Utility
	struct UintMinMax {
		uint32_t min;
		uint32_t max;
		uint32_t pad[2];
	};

	struct Vector3MinMax {
		Vector3 min;
		uint32_t pad1;
		Vector3 max;
		uint32_t pad2;
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
		uint32_t pad[2];
	};
	/*
	struct Particle {
		struct Float3MinMax {
			float3 min;
			uint pad1;
			float3 max;
			uint pad2;
		} scaleRange;
		struct ParticleLifeTime {
			uint time;
			uint maxTime;
			uint2 pad;
		} particleLifeTime;
		float4 color;
		float3 scale;
		uint textureInidex;

		float32_t rotateVelocity;
		uint isAlive;
		float32_t rotate;
		uint pad1;

		float3 translate;
		uint pad2;
		float3 velocity;
		uint pad3;
	} Element;
	*/

	struct Particle {
		Vector3MinMax scaleRange;
		ParticleLifeTime particleLifeTime;
		Vector4 color;

		Vector3 scale;
		uint32_t textureInidex;
		
		float rotateVelocity;
		uint32_t isAlive;
		float rotate;
		uint32_t pad1;
		
		Vector3 translate;
		uint32_t pad2;
		Vector3 velocity;
		uint32_t pad3;
	};

	// hlsli側も変更すること

	// エミッターの生成範囲と生成場所
	struct EmitterAABB {
		Vector3MinMax area;
	};

	struct EmitterSphere {
		float radius;
		Vector3 pad;
	};

	struct EmitterArea {
		EmitterAABB aabb;
		EmitterSphere sphere;
		Vector3 position;
		uint32_t type;
	};

	// エミッターの生成範囲と生成場所

	// パーティクルのスケール
	struct ScaleAnimation {
		Vector3StartEnd range;
	};

	// パーティクルの回転	
	struct RotateAnimation {
		float rotate;
		uint32_t pad[3];
	};

	// パーティクルの移動
	struct Velocity3D {
		Vector3MinMax range;
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

		uint32_t pad;
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

}