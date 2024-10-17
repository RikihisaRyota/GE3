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
#include "Engine/Math/Quaternion.h"

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
		uint32_t isEmitterLife;
		// エミッターが死んでからカウントダウンを開始するか
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
		uint pad;
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
		// パーティクルのローカルTranslate
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

	struct Particle {
		TriangleInfo triangleInfo;

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

		Translate translate;

		Matrix4x4 matWorld;

		ParticleAttributes collisionInfo;

		ParticleParent parent;

		Vector3 velocity;
		float pad;
	};

	// hlsli側も変更すること

	// エミッターの生成範囲と生成場所
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

	// エミッターの生成範囲と生成場所

	// パーティクルのスケール
	struct ScaleAnimation {
		Vector3StartEnd range;
		// x,y,z同じサイズ
		uint32_t isUniformScale;
		// イージングしてもサイズ変更無し
		uint32_t isStaticSize;
		Vector2 pad;
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
		uint32_t isStaticColor;
		Vector3 pad;
	};

	// エミッターの生成間隔
	struct EmitterFrequency {
		// インターバル
		int32_t interval;
		// ループするか
		int32_t isLoop;
		// エミッターが生きている間パーティクルも生きる
		int32_t emitterLife;
		// 一度しか出さないか
		int32_t isOnce;
	};

	struct EmitterTime {
		int32_t particleTime = 0;
		int32_t emitterTime = 0;
		Vector2 pad;
	};

	// パーティクルのランダム寿命
	struct ParticleLifeSpan {
		UintMinMax range;
		uint32_t isEmitterLife;
		// エミッターが死んでからカウントダウンを開始するか
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
		Vector3 scale = {1.0f,1.0f,1.0f};
		float pad;
		Vector3 translate;
		float pad1;
		Quaternion rotate = { 0.0f,0.0f,0.0f,1.0f };
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

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel startModel;

		Matrix4x4 startModelWorldMatrix;

		EmitterModel endModel;

		Matrix4x4 endModelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive = true;

		// クラス内でstatic宣言されたメンバ変数のサイズは0
		static int32_t staticEmitterCount;
	};

	struct TransformModelEmitterForGPU {
		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel startModel;

		Matrix4x4 startModelWorldMatrix;

		EmitterModel endModel;

		Matrix4x4 endModelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount = -1;

		uint32_t isAlive = false;

		float pad;
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

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		Matrix4x4 modelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive = true;

		float pad;

		// クラス内でstatic宣言されたメンバ変数のサイズは0
		static int32_t staticEmitterCount;
	};

	struct TransformAreaEmitterForGPU {

		EmitterArea emitterArea;

		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		Matrix4x4 modelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount = -1;

		uint32_t isAlive = false;

		float pad;
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

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t numCreate;

		uint32_t isAlive = true;

		int32_t emitterCount;

		// クラス内でstatic宣言されたメンバ変数のサイズは0
		static int32_t staticEmitterCount;
	};

	struct MeshEmitterForGPU {
		Translate translate;

		EmitterLocalTransform localTransform;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t numCreate;

		uint32_t isAlive = false;

		int32_t emitterCount = -1;
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

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t isAlive = true;

		int32_t emitterCount;

		// クラス内でstatic宣言されたメンバ変数のサイズは0
		static int32_t staticEmitterCount;
	};

	struct VertexEmitterForGPU {
		Translate translate;

		EmitterLocalTransform localTransform;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

		uint32_t textureIndex;

		uint32_t isAlive = false;

		int32_t emitterCount = -1;

		float pad;
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

		EmitterParent parent;

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

		EmitterParent parent;
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
		Vector3MinMax externalForce;
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

		Vector2 pad;

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
		uint32_t maxCreateParticleNum;
		uint32_t emitterType;

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
	void DrawColor(GPUParticleShaderStructs::EmitterColor& color);
	void DrawFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency);
	void DrawParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan);
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

	void LoadColor(GPUParticleShaderStructs::EmitterColor& color);
	void SaveColor(GPUParticleShaderStructs::EmitterColor& color);

	void LoadFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency);
	void SaveFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency);

	void LoadParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan);
	void SaveParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan);

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

	// エミッターカウントを共有しない
	void NonSharedCopy(GPUParticleShaderStructs::EmitterForCPU& dst,const GPUParticleShaderStructs::EmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::VertexEmitterForCPU& dst,const GPUParticleShaderStructs::VertexEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::MeshEmitterForCPU& dst,const GPUParticleShaderStructs::MeshEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::TransformAreaEmitterForCPU& dst,const GPUParticleShaderStructs::TransformAreaEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::TransformModelEmitterForCPU& dst,const GPUParticleShaderStructs::TransformModelEmitterForCPU& src);
	void NonSharedCopy(GPUParticleShaderStructs::FieldForCPU& dst,const GPUParticleShaderStructs::FieldForCPU& src);
	
	// CPUからGPUへ
	void Copy(GPUParticleShaderStructs::EmitterForGPU& dst, const GPUParticleShaderStructs::EmitterForCPU& src,const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::VertexEmitterForGPU& dst,const GPUParticleShaderStructs::VertexEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::MeshEmitterForGPU& dst,const GPUParticleShaderStructs::MeshEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::TransformAreaEmitterForGPU& dst,const GPUParticleShaderStructs::TransformAreaEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::TransformModelEmitterForGPU& dst,const GPUParticleShaderStructs::TransformModelEmitterForCPU& src, const Matrix4x4& parent);
	void Copy(GPUParticleShaderStructs::FieldForGPU& dst,const GPUParticleShaderStructs::FieldForCPU& src);
}