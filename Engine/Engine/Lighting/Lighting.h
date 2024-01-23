#pragma once

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
		float sharpness;
	};
public:
	static Lighting* GetInstance();

	void Initialize();

	void Update();

	const UploadBuffer& GetDirectionLightBuffer() const { return directionLightBuffer_; }
	const UploadBuffer& GetPointLightBuffer() const { return pointLightBuffer_; }
private:
	UploadBuffer directionLightBuffer_;
	DirectionLight* directionLightData_;

	UploadBuffer pointLightBuffer_;
	PointLight* pointLightData_;
};