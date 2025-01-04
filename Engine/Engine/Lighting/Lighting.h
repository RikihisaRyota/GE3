#pragma once
/**
 * @file Lighting.h
 * @brief ライティングに必要な物をまとめている
 */
#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Vector3.h"

class Lighting {
public:
	struct DirectionLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
		float sharpness;
	};
	struct PointLight {
		Vector4 color;
		Vector3 position;
		float intensity;
		float radius;
		float decay;
		float sharpness;
		float pad;
	};

public:
	// インスタンス
	static Lighting* GetInstance();
	// 初期化
	void Initialize();
	// 更新
	void Update();

	// Getter
	const UploadBuffer& GetDirectionLightBuffer() const { return directionLightBuffer_; }
	DirectionLight* GetDirectionLight() const { return directionLightData_; }
	const UploadBuffer& GetPointLightBuffer() const { return pointLightBuffer_; }
	PointLight* GetPointLight() const { return pointLightData_; }
private:
	UploadBuffer directionLightBuffer_;
	DirectionLight* directionLightData_;

	UploadBuffer pointLightBuffer_;
	PointLight* pointLightData_;
};