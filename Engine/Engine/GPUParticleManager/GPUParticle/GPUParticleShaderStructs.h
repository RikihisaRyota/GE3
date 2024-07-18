#pragma once


#include <string>
#include <limits>
#include <tuple>

#include <d3d12.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"

namespace GPUParticleShaderStructs {
	// hlsli側も変更するように
	static const UINT ComputeThreadBlockSize = 1024;
	static const UINT MeshComputeThreadBlockSize = 1024;
	static const UINT MaxParticleShouldBeSquare = 22;
	static const UINT MaxParticleNum = 1 << MaxParticleShouldBeSquare;
	static const UINT MaxEmitterNum = 1024;
	static const UINT MaxFieldNum = 1024;
	static const UINT MaxBulletNum = 10;
	static const UINT MaxProcessNum = 1;


#pragma region Utility
	struct UintMinMax {
		uint32_t min;
		uint32_t max;
		uint32_t pad[2];
	};

	struct FloatMinMax {
		float min;
		float max;
		Vector2 pad;
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
	*

struct Particle
{
	struct Float3MinMax
	{
		float3 min;
		uint pad1;
		float3 max;
		uint pad2;
	} scaleRange;
	struct ParticleLifeTime
	{
		uint time;
		uint maxTime;
		uint2 pad;
	} particleLifeTime;
	struct Float4MinMax
	{
		float4 min;
		float4 max;
	} colorRange;
	float4 color;
	float3 scale;
	uint textureIndex;
	float rotateVelocity;
	float rotate;
	uint isAlive;
	uint pad1;
	float3 translate;
	uint pad2;
	float3 velocity;
	uint pad3;
	float4x4 worldMatrix; // row_major
	struct ParticleAttributes
	{
		uint mask;
		uint attribute;
		float2 pad;
	} collisionInfo;
} Element;


	*/
	struct ParticleAttributes {
		uint32_t mask;
		uint32_t attribute;
		Vector2 pad;
	};

	struct Particle {
		Vector3MinMax scaleRange;
		ParticleLifeTime particleLifeTime;

		Vector4MinMax colorRange;
		Vector4 color;

		Vector3 scale;
		uint32_t textureInidex;

		float rotateVelocity;
		float rotate;
		uint32_t isAlive;
		uint32_t isHit;

		Vector3 translate;
		uint32_t pad2;

		Vector3 velocity;
		uint32_t pad3;

		Matrix4x4 matWorld;

		ParticleAttributes collisionInfo;
	};

	// hlsli側も変更すること

	// エミッターの生成範囲と生成場所
	struct EmitterAABB {
		Vector3MinMax area;
		Vector3 position;
		float pad;
	};

	struct EmitterSphere {
		float radius;
		Vector3 position;
		FloatMinMax distanceFactor;
	};

	struct EmitterSegment {
		Vector3 origin;
		float pad;
		Vector3 diff;
		float pad1;
	};
	struct EmitterCapsule {
		EmitterSegment segment;
		FloatMinMax distanceFactor;
		float radius;
		Vector3 pad;
	};
	enum Type {
		kAABB,
		kSphere,
		kCapsule,
		kFigureCount,
	};
	struct EmitterArea {
		EmitterAABB aabb;
		EmitterSphere sphere;
		EmitterCapsule capsule;
		uint32_t type;
		Vector3 pad;
	};

	// エミッターの生成範囲と生成場所

	// パーティクルのスケール
	struct ScaleAnimation {
		Vector3StartEnd range;
		uint32_t isSame;
		Vector3 pad;
	};

	// パーティクルの回転	
	struct RotateAnimation {
		FloatMinMax initializeAngle;
		FloatMinMax rotateSpeed;
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
		uint32_t interval;
		uint32_t isLoop;
		uint32_t emitterLife;
		uint32_t pad;
	};

	struct EmitterTime {
		uint32_t particleTime = 0;
		uint32_t emitterTime = 0;
		Vector2 pad;
	};

	// パーティクルのランダム寿命
	struct ParticleLifeSpan {
		UintMinMax range;
	};

	struct MeshEmitter {
		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		uint32_t textureIndex;

		Vector3 pad;
	};

	struct MeshEmitterDesc {
		MeshEmitter emitter;
		uint32_t numCreate;
	};

	struct VertexEmitterDesc {
		MeshEmitter emitter;
	};

	// エミッター(CPUとGPUある)
	struct EmitterForCPU {
		EmitterForCPU() {
			if (staticEmitterCount == (std::numeric_limits<int32_t>::max)()) {
				staticEmitterCount = 0;
			}
			emitterCount = staticEmitterCount;
			staticEmitterCount++;
		}
		EmitterArea emitterArea;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		uint32_t textureIndex;

		uint32_t createParticleNum;

		uint32_t isAlive = true;

		int32_t emitterCount;

		ParticleAttributes collisionInfo;

		// クラス内でstatic宣言されたメンバ変数のサイズは0
		static int32_t staticEmitterCount;
	};

	// エミッター(CPUとGPUある)
	struct EmitterForGPU {

		EmitterArea emitterArea;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		uint32_t textureIndex;

		uint32_t createParticleNum;

		uint32_t isAlive = false;

		int32_t emitterCount = -1;

		ParticleAttributes collisionInfo;
	};

	enum FieldType {
		kAttraction,
		kExternalForce,
		kFieldCount,
	};

	struct Attraction {
		float attraction;
		Vector3 pad;
	};


	struct ExternalForce {
		Vector3 externalForce;
		float pad;
	};

	struct Field {
		Attraction attraction;
		ExternalForce externalForce;
		uint32_t type;
		Vector3 pad;
	};

	struct FieldFrequency {
		uint32_t isLoop;
		uint32_t lifeCount;
		Vector2 pad;
	};

	struct FieldForCPU {
		FieldForCPU() {
			if (staticFieldCount == (std::numeric_limits<int32_t>::max)()) {
				staticFieldCount = 0;
			}
			fieldCount = staticFieldCount;
			staticFieldCount++;
		}

		Field field;

		EmitterArea fieldArea;

		FieldFrequency frequency;

		ParticleAttributes collisionInfo;

		uint32_t isAlive = true;

		int32_t fieldCount;

		// クラス内でstatic宣言されたメンバ変数のサイズは0
		static int32_t staticFieldCount;
	};

	struct FieldForGPU {

		Field field;

		EmitterArea fieldArea;

		FieldFrequency frequency;

		ParticleAttributes collisionInfo;

		uint32_t isAlive = false;

		int32_t fieldCount = -1;

		Vector2 pad;
	};


	struct CreateParticle {
		uint32_t emitterIndex;
		int32_t createParticleNum;
	};

	// 弾
	struct BulletForGPU {
		ParticleAttributes collisionInfo;
		struct Bullet {
			Vector3 position;
			float radius;
			float speed;
			Vector3 pad;
		} bullet;
		struct Emitter {
			ParticleLifeSpan particleLifeSpan;
		}emitter;
	};

	struct IndirectCommand {
		struct SRV {
			D3D12_GPU_VIRTUAL_ADDRESS particleSRV;
			D3D12_GPU_VIRTUAL_ADDRESS drawIndexSRV;
		};
		SRV srv;
		D3D12_DRAW_INDEXED_ARGUMENTS drawIndex;
	};

	void EmitterEditor(const std::string name, std::tuple<bool*, EmitterForCPU*> emitter);
	void EmitterEditor(const std::string name, std::tuple<bool*, MeshEmitterDesc*> desc);
	void EmitterEditor(const std::string name, std::tuple<bool*, VertexEmitterDesc*> desc);
	void EmitterEditor(const std::string name, std::tuple<bool*, FieldForCPU*> desc);

	void Debug(const std::string name, EmitterForCPU& emitter);
	void Debug(const std::string name, MeshEmitterDesc& desc);
	void Debug(const std::string name, VertexEmitterDesc& desc);
	void Debug(const std::string name, FieldForCPU& desc);

	void DebugDraw(const EmitterForCPU& emitter);
	void DebugDraw(const FieldForCPU& emitter);

	void Update();

	void Save(const std::string name, EmitterForCPU& emitter);
	void Load(const std::string name, EmitterForCPU& emitter);
	void Save(const std::string name, MeshEmitterDesc& desc);
	void Load(const std::string name, MeshEmitterDesc& desc);
	void Save(const std::string name, VertexEmitterDesc& desc);
	void Load(const std::string name, VertexEmitterDesc& desc);
	void Save(const std::string name, FieldForCPU& desc);
	void Load(const std::string name, FieldForCPU& desc);


	void SaveMinMax(GPUParticleShaderStructs::UintMinMax& startEnd);

	void LoadMinMax(GPUParticleShaderStructs::UintMinMax& startEnd);

	void SaveMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd);

	void LoadMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd);

	void SaveMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd);

	void LoadMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd);

	void DrawMinMax(GPUParticleShaderStructs::UintMinMax& startEnd, float v_speed = 1.0f, int v_min = 0, int v_max = 0);

	void DrawMinMax(GPUParticleShaderStructs::FloatMinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawColorMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd);

	void LoadStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd);

	void SaveStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd);

	void LoadStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd);

	void SaveStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd);

	void DrawStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawColor(GPUParticleShaderStructs::Vector4StartEnd& startEnd);
}