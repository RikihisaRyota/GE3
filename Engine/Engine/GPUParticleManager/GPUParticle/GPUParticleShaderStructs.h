#pragma once

#ifdef __cplusplus
/**
 * @file GPUParticleShaderStructs.h
 * @brief GPUParticleShaderStructs
 */
// Japanese is prohibited as it is included in hlsl
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
#include "Engine/Math/Quaternion.h"

#endif 

namespace GPUParticleShaderStructs {
#ifndef __cplusplus
#define Vector2 float32_t2
#define Vector3 float32_t3
#define Vector4 float32_t4
#define Matrix4x4 float32_t4x4
#define Quaternion float32_t4
#define UINT uint
#endif
	static const UINT ComputeThreadBlockSize = 1024;
	static const UINT MeshComputeThreadBlockSize = 1024;
	static const UINT MaxParticleShouldBeSquare = 20;
	static const UINT MaxParticleNum = 1U << MaxParticleShouldBeSquare;
	static const UINT DivisionNum = 2;
	static const UINT DivisionParticleNum = MaxParticleNum / DivisionNum;
	static const UINT MaxEmitterNum = 1024;
	static const UINT MaxFieldNum = 1024;
	static const UINT MaxBulletNum = 10;
	static const UINT MaxProcessNum = 1;
	static const UINT TrailsRange = 512;
	static const UINT MaxTrailsNum = 1U << 16;
	static const UINT MaxTrailsTotal = MaxTrailsNum * TrailsRange;

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

	struct ParticleLifeTime {
		uint32_t time;
		uint32_t maxTime;
		uint32_t isEmitterLife;
		uint32_t isCountDown;
	};
	/*
	*

struct Particle
{
	struct TriangleInfo {
		float3 vertex;
		float3 weight;
	} info;
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
		uint isEmitterLife;
		uint isCountDown;
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
	int isAlive;
	int isHit;
	struct Translate {
		Float3MinMax easing;
		float3 translate;
		int isEasing;
		float radius;
		float attraction;
		float2 pad;
	}translate;
	float4x4 worldMatrix; // row_major
	struct ParticleAttributes
	{
		uint mask;
		uint attribute;
		float2 pad;
	} collisionInfo;
	struct ParticleParent{
		uint isParent;
		uint emitterType;
		uint emitterCount;
		uint pad;
	}parent;

	float3 velocity;
	uint pad3;
} Element;


*/


	struct ParticleAttributes {
		uint32_t attribute;
		uint32_t mask;
		Vector2 pad;
	};
	struct Translate {
		Vector3MinMax easing;
		Vector3 translate;
		uint32_t isEasing;

		float radius;
		float attraction;
		Vector2 pad;
	};

	struct ParticleParent {
		uint32_t isParent;
		uint32_t emitterType;
		uint32_t emitterCount;
		uint32_t pad;
	};

	struct TriangleInfo {
		Vector3 vertex;
		//float pad;
		Vector3 weight;
		//float pad1;
	};

	struct TrailsData {
#ifdef __cplusplus
		TrailsData() {
			trailsIndex = -1;
			isAlive = 0;
		};
#endif
		uint32_t particleIndex;
		int32_t trailsIndex;
		uint32_t startIndex;
		uint32_t endIndex;

		uint32_t currentIndex;
		uint32_t textureIndex;
		uint32_t interval;
		uint32_t time;


		int32_t loopNum;
		uint32_t isAlive;
		float width;
		float lifeLimit;
	};

	struct TrailsVertex {
		Vector3 position;
		Vector2 uv;
	};

	struct TrailsVertexData {
		TrailsVertex vertex[3];
	};

	struct TrailsPosition {
		Vector3 position;
		TrailsVertex vertex[2];
	};

	struct TrailsHead {
		uint32_t headIndex;
	};

	struct TrailsIndex {
		int32_t positionIndex;
		int32_t trailsIndex;
	};

	struct EmitterTrails {
#ifdef __cplusplus
		EmitterTrails() {
			interval = 5;
			width = 0.5f;
			lifeLimit = 1.0f;
		};
#endif

		uint32_t isTrails;
		uint32_t textureIndex;
		uint32_t interval;
		float width;

		float lifeLimit;
		float pad[3];
	};

	struct Particle {
		TriangleInfo triangleInfo;

		Vector3MinMax scaleRange;
		Vector3 medScale;
		uint32_t isMedPoint;
		ParticleLifeTime particleLifeTime;

		Vector4MinMax colorRange;
		Vector4 color;

		Vector3 scale;
		uint32_t textureIndex;

		Translate translate;

		Matrix4x4 matWorld;

		ParticleAttributes collisionInfo;

		ParticleParent parent;

		Vector3 velocity;
		uint32_t isHit;

		Vector3 acceleration;
		uint32_t isAlive;

		float rotateVelocity;
		float rotate;
		float pad1[2];
	};


	struct EmitterAABB {
		Vector3MinMax area;
	};

	struct EmitterSphere {
		float radius;
		Vector3 pad;
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
		Vector3 position;
		uint32_t type;
	};


	struct ScaleAnimation {
		Vector3StartEnd range;
		Vector3MinMax mediumRange;
		uint32_t isUniformScale;
		uint32_t isStaticSize;
		uint32_t isMedPoint;
		uint32_t pad;
	};

	struct RotateAnimation {
		FloatMinMax initializeAngle;
		FloatMinMax rotateSpeed;
	};

	struct Velocity3D {
		Vector3MinMax range;
	};

	struct Acceleration3D {
		Vector3MinMax range;
	};

	struct EmitterColor {
		Vector4StartEnd range;
		uint32_t isStaticColor;
		Vector3 pad;
	};

	struct EmitterFrequency {
		int32_t interval;
		int32_t isLoop;
		int32_t emitterLife;
		int32_t isOnce;
	};

	struct EmitterTime {
#ifdef __cplusplus
		EmitterTime() {
			particleTime = 0;
			emitterTime = 0;
		};
#endif
		int32_t particleTime;
		int32_t emitterTime;
		Vector2 pad;
	};

	struct ParticleLifeSpan {
#ifdef __cplusplus
		ParticleLifeSpan() {
			isCountDown = false;
		};
#endif
		UintMinMax range;
		uint32_t isEmitterLife;
		uint32_t isCountDown;
		Vector2 pad;
	};

	enum EmitterType {
		kEmitter,
		kVertexEmitter,
		kMeshEmitter,
		kTransformModelEmitter,
		kTransformAreaEmitter,
		kCount,
	};

	struct EmitterParent {
		Matrix4x4 worldMatrix;
		uint32_t isParent;
		uint32_t emitterType;
		Vector2 pad;
	};

	struct EmitterModel {
		uint32_t vertexBufferIndex;
		uint32_t indexBufferIndex;
		uint32_t vertexCount;
		uint32_t indexCount;
	};

	struct EmitterLocalTransform {
#ifdef __cplusplus
		EmitterLocalTransform() {
			scale = { 1.0f,1.0f,1.0f };
			rotate = { 0.0f,0.0f,0.0f,1.0f };
		};
#endif

		Vector3 scale;
		float pad;
		Vector3 translate;
		float pad1;
		Quaternion rotate;
	};

	struct TransformModelEmitterForGPU {
#ifdef __cplusplus
		TransformModelEmitterForGPU() {
			emitterCount = -1;
			isAlive = false;
		};
#endif
		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel startModel;

		Matrix4x4 startModelWorldMatrix;

		EmitterModel endModel;

		Matrix4x4 endModelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive;

		float pad;
	};

	struct TransformAreaEmitterForGPU {
#ifdef __cplusplus

		TransformAreaEmitterForGPU() {
			emitterCount = -1;

			isAlive = false;
		};
#endif

		EmitterArea emitterArea;

		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		Matrix4x4 modelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive;

		float pad;
	};

	struct MeshEmitterForGPU {
#ifdef __cplusplus

		MeshEmitterForGPU() {
			isAlive = false;

			emitterCount = -1;
		};
#endif

		Translate translate;

		EmitterLocalTransform localTransform;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t numCreate;

		uint32_t isAlive;

		int32_t emitterCount;
	};

	struct VertexEmitterForGPU {
#ifdef __cplusplus

		VertexEmitterForGPU() {
			isAlive = false;

			emitterCount = -1;
		};
#endif

		Translate translate;

		EmitterLocalTransform localTransform;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t isAlive;

		int32_t emitterCount;

		float pad;
	};

	struct EmitterForGPU {
#ifdef __cplusplus

		EmitterForGPU() {
			isAlive = false;

			emitterCount = -1;
		};
#endif

		EmitterArea emitterArea;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		uint32_t textureIndex;

		uint32_t createParticleNum;

		uint32_t isAlive;

		int32_t emitterCount;

		ParticleAttributes collisionInfo;

		EmitterParent parent;
	};

	enum FieldType {
		kAttraction,
		kExternalForce,
		kVelocityRotateForce,
		kPositionRotateForce,
		kFieldCount,
	};

	struct Attraction {
		float attraction;
		Vector3 pad;
	};


	struct ExternalForce {
		Vector3MinMax externalForce;
	};

	struct VelocityRotateForce {
		Vector3 direction;
		float rotateSpeed;
	};

	struct PositionRotateForce {
		Vector3 direction;
		float rotateSpeed;
	};

	struct Field {
		Attraction attraction;
		ExternalForce externalForce;
		VelocityRotateForce velocityRotateForce;
		PositionRotateForce positionRotateForce;
		uint32_t type;
		Vector3 pad;
	};

	struct FieldFrequency {
		uint32_t isLoop;
		uint32_t lifeCount;
		Vector2 pad;
	};



	struct FieldForGPU {
#ifdef __cplusplus
		FieldForGPU() {
			isAlive = false;

			fieldCount = -1;
		};
#endif
		Field field;

		EmitterArea fieldArea;

		FieldFrequency frequency;

		ParticleAttributes collisionInfo;

		uint32_t isAlive;

		int32_t fieldCount;

		Vector2 pad;
	};


	struct CreateParticle {
		uint32_t emitterIndex;
		int32_t createParticleNum;
		uint32_t maxCreateParticleNum;
		uint32_t emitterType;

	};

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

	struct DrawIndex {
		int32_t directIndex;
		int32_t computeIndex;
	};

#ifdef __cplusplus
	struct IndirectCommand {
		struct SRV {
			D3D12_GPU_VIRTUAL_ADDRESS directParticleSRV;
			D3D12_GPU_VIRTUAL_ADDRESS computeParticleSRV;
			D3D12_GPU_VIRTUAL_ADDRESS drawIndexSRV;
		};
		SRV srv;
		D3D12_DRAW_INDEXED_ARGUMENTS drawIndex;
	};

	struct TrailsCommand {
		struct SRV {
			D3D12_GPU_VIRTUAL_ADDRESS trailsData;
			D3D12_GPU_VIRTUAL_ADDRESS trailsPosition;
			D3D12_GPU_VIRTUAL_ADDRESS counterBuffer;
			D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer;
			D3D12_GPU_VIRTUAL_ADDRESS instanceCount;
		};
		SRV srv;
		D3D12_DRAW_ARGUMENTS drawIndex;
	};

	struct TransformModelEmitterForCPU {
		TransformModelEmitterForCPU() {
			if (staticEmitterCount == (std::numeric_limits<int32_t>::max)()) {
				staticEmitterCount = 0;
			}
			emitterCount = staticEmitterCount;
			staticEmitterCount++;
		}
		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel startModel;

		Matrix4x4 startModelWorldMatrix;

		EmitterModel endModel;

		Matrix4x4 endModelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive = true;

		static int32_t staticEmitterCount;
	};

	struct TransformAreaEmitterForCPU {
		TransformAreaEmitterForCPU() {
			if (staticEmitterCount == (std::numeric_limits<int32_t>::max)()) {
				staticEmitterCount = 0;
			}
			emitterCount = staticEmitterCount;
			staticEmitterCount++;
		}

		EmitterArea emitterArea;

		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		Matrix4x4 modelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive = true;

		float pad;

		static int32_t staticEmitterCount;
	};

	struct MeshEmitterForCPU {
		MeshEmitterForCPU() {
			if (staticEmitterCount == (std::numeric_limits<int32_t>::max)()) {
				staticEmitterCount = 0;
			}
			emitterCount = staticEmitterCount;
			staticEmitterCount++;
		}
		Translate translate;

		EmitterLocalTransform localTransform;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t numCreate;

		uint32_t isAlive = true;

		int32_t emitterCount;

		static int32_t staticEmitterCount;
	};

	struct VertexEmitterForCPU {
		VertexEmitterForCPU() {
			if (staticEmitterCount == (std::numeric_limits<int32_t>::max)()) {
				staticEmitterCount = 0;
			}
			emitterCount = staticEmitterCount;
			staticEmitterCount++;
		}
		Translate translate;

		EmitterLocalTransform localTransform;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t isAlive = true;

		int32_t emitterCount;

		static int32_t staticEmitterCount;
	};


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

		Acceleration3D acceleration;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		EmitterTrails emitterTrails;

		uint32_t textureIndex;

		uint32_t createParticleNum;

		uint32_t isAlive = true;

		int32_t emitterCount;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		static int32_t staticEmitterCount;
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

		Vector2 pad;

		static int32_t staticFieldCount;
	};

	void EmitterEditor(const std::string name, std::tuple<bool*, EmitterForCPU*, Matrix4x4> emitter);
	void EmitterEditor(const std::string name, std::tuple<bool*, MeshEmitterForCPU*> emitter);
	void EmitterEditor(const std::string name, std::tuple<bool*, VertexEmitterForCPU*> emitter);
	void EmitterEditor(const std::string name, std::tuple<bool*, TransformModelEmitterForCPU*> emitter);
	void EmitterEditor(const std::string name, std::tuple<bool*, TransformAreaEmitterForCPU*> emitter);
	void EmitterEditor(const std::string name, std::tuple<bool*, FieldForCPU*> emitter);

	void Debug(const std::string name, EmitterForCPU& emitter, const Matrix4x4& parent = Matrix4x4());
	void Debug(const std::string name, MeshEmitterForCPU& emitter);
	void Debug(const std::string name, VertexEmitterForCPU& emitter);
	void Debug(const std::string name, TransformModelEmitterForCPU& emitter);
	void Debug(const std::string name, TransformAreaEmitterForCPU& emitter);
	void Debug(const std::string name, FieldForCPU& emitter);

	void DebugDraw(const EmitterForCPU& emitter);
	void DebugDraw(const FieldForCPU& emitter);

	void Update();

	void Save(const std::string name, EmitterForCPU& emitter);
	void Load(const std::string name, EmitterForCPU& emitter);
	void Save(const std::string name, MeshEmitterForCPU& desc);
	void Load(const std::string name, MeshEmitterForCPU& desc);
	void Save(const std::string name, VertexEmitterForCPU& desc);
	void Load(const std::string name, VertexEmitterForCPU& desc);
	void Save(const std::string name, TransformModelEmitterForCPU& emitter);
	void Load(const std::string name, TransformModelEmitterForCPU& emitter);
	void Save(const std::string name, TransformAreaEmitterForCPU& emitter);
	void Load(const std::string name, TransformAreaEmitterForCPU& emitter);
	void Save(const std::string name, FieldForCPU& desc);
	void Load(const std::string name, FieldForCPU& desc);


	void SaveMinMax(GPUParticleShaderStructs::UintMinMax& startEnd);
	void LoadMinMax(GPUParticleShaderStructs::UintMinMax& startEnd);
	void SaveMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd);
	void LoadMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd);
	void SaveMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd);
	void LoadMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd);

	void LoadStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd);
	void SaveStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd);
	void LoadStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd);
	void SaveStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd);

	void DrawMinMax(GPUParticleShaderStructs::UintMinMax& startEnd, float v_speed = 1.0f, int v_min = 0, int v_max = 0);
	void DrawMinMax(GPUParticleShaderStructs::FloatMinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
	void DrawMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
	void DrawMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
	void DrawStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);

	void DrawColorMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd);

	void DrawLocalTranslate(GPUParticleShaderStructs::EmitterLocalTransform& localTransform);
	void DrawTranslate(GPUParticleShaderStructs::Translate& translate);
	void DrawColor(GPUParticleShaderStructs::Vector4StartEnd& startEnd);
	void DrawArea(GPUParticleShaderStructs::EmitterArea& area);
	void DrawScale(GPUParticleShaderStructs::ScaleAnimation& scale);
	void DrawRotate(GPUParticleShaderStructs::RotateAnimation& rotate);
	void DrawVelocity(GPUParticleShaderStructs::Velocity3D& velocity);
	void DrawAcceleration(GPUParticleShaderStructs::Acceleration3D& acceleration);
	void DrawColor(GPUParticleShaderStructs::EmitterColor& color);
	void DrawFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency);
	void DrawParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan);
	void DrawTrails(GPUParticleShaderStructs::EmitterTrails& emitterTrails);
	void DrawTextureHandle(uint32_t& texture);
	void DrawCreateParticleNum(uint32_t& createParticleNum);
	void DrawParent(uint32_t& parent);
	void DrawCollisionInfo(GPUParticleShaderStructs::ParticleAttributes& particleAttributes);
	void DrawField(GPUParticleShaderStructs::Field& fierd);
	void DrawFieldFrequency(GPUParticleShaderStructs::FieldFrequency& fieldFrequency);

	void LoadLocalTransform(GPUParticleShaderStructs::EmitterLocalTransform& local);
	void SaveLocalTransform(GPUParticleShaderStructs::EmitterLocalTransform& local);

	void LoadTranslate(GPUParticleShaderStructs::Translate& translate);
	void SaveTranslate(GPUParticleShaderStructs::Translate& translate);

	void LoadArea(GPUParticleShaderStructs::EmitterArea& area);
	void SaveArea(GPUParticleShaderStructs::EmitterArea& area);

	void LoadScale(GPUParticleShaderStructs::ScaleAnimation& scale);
	void SaveScale(GPUParticleShaderStructs::ScaleAnimation& scale);

	void LoadRotate(GPUParticleShaderStructs::RotateAnimation& rotate);
	void SaveRotate(GPUParticleShaderStructs::RotateAnimation& rotate);

	void LoadVelocity(GPUParticleShaderStructs::Velocity3D& velocity);
	void SaveVelocity(GPUParticleShaderStructs::Velocity3D& velocity);

	void LoadAcceleration(GPUParticleShaderStructs::Acceleration3D& acceleration);
	void SaveAcceleration(GPUParticleShaderStructs::Acceleration3D& acceleration);

	void LoadColor(GPUParticleShaderStructs::EmitterColor& color);
	void SaveColor(GPUParticleShaderStructs::EmitterColor& color);

	void LoadFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency);
	void SaveFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency);

	void LoadParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan);
	void SaveParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan);

	void LoadTrails(GPUParticleShaderStructs::EmitterTrails& emitterTrails);
	void SaveTrails(GPUParticleShaderStructs::EmitterTrails& emitterTrails);

	void LoadTextureHandle(uint32_t& texture);
	void SaveTextureHandle(uint32_t& texture);

	void LoadCreateParticleNum(uint32_t& createParticleNum);
	void SaveCreateParticleNum(uint32_t& createParticleNum);

	void LoadCollisionInfo(GPUParticleShaderStructs::ParticleAttributes& particleAttributes);
	void SaveCollisionInfo(GPUParticleShaderStructs::ParticleAttributes& particleAttributes);

	void LoadParent(GPUParticleShaderStructs::EmitterParent& parent);
	void SaveParent(GPUParticleShaderStructs::EmitterParent& parent);

	void LoadField(GPUParticleShaderStructs::Field& fierd);
	void SaveField(GPUParticleShaderStructs::Field& fierd);

	void LoadFieldArea(GPUParticleShaderStructs::EmitterArea& area);
	void SaveFieldArea(GPUParticleShaderStructs::EmitterArea& area);

	void LoadFieldFrequency(GPUParticleShaderStructs::FieldFrequency& fieldFrequency);
	void SaveFieldFrequency(GPUParticleShaderStructs::FieldFrequency& fieldFrequency);

	// copy not sharing index
	void NonSharedCopy(GPUParticleShaderStructs::EmitterForCPU& dst, const GPUParticleShaderStructs::EmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::VertexEmitterForCPU& dst, const GPUParticleShaderStructs::VertexEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::MeshEmitterForCPU& dst, const GPUParticleShaderStructs::MeshEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::TransformAreaEmitterForCPU& dst, const GPUParticleShaderStructs::TransformAreaEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::TransformModelEmitterForCPU& dst, const GPUParticleShaderStructs::TransformModelEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::FieldForCPU& dst, const GPUParticleShaderStructs::FieldForCPU& src);

	void Copy(GPUParticleShaderStructs::EmitterForGPU& dst, const GPUParticleShaderStructs::EmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::VertexEmitterForGPU& dst, const GPUParticleShaderStructs::VertexEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::MeshEmitterForGPU& dst, const GPUParticleShaderStructs::MeshEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::TransformAreaEmitterForGPU& dst, const GPUParticleShaderStructs::TransformAreaEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::TransformModelEmitterForGPU& dst, const GPUParticleShaderStructs::TransformModelEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::FieldForGPU& dst, const GPUParticleShaderStructs::FieldForCPU& src);
#endif  // __cpluspulus
}
