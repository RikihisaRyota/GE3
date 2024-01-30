#pragma once
#include <filesystem>
#include <cstdint>
#include <vector>
#include <memory>

#include <d3d12.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"

#include "Engine/Math/Vector4.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Matrix4x4.h"


class Sprite {
private:
	struct Material {
		Vector4 color;
	};

	struct Vertex {
		Vector3 position;
		Vector2 texcoord;
	};

public:
	void Create(const std::filesystem::path& modelPath);

	const std::filesystem::path& GetName() const { return name_; }
	const TextureHandle& GetTextureHandle() const { return textureHandle_; }
	const UploadBuffer& GetMaterialBuffer()const { return materialBuffer_; }
	const UploadBuffer& GetWorldMatBuffer()const { return worldMatBuffer_; }
	void SetPosition(const Vector2 pos);
private:
	static const Matrix4x4 sMatProjection;
	void LoadFile(const std::filesystem::path& modelPath);
	
	Vertex* vertex_;
	TextureHandle textureHandle_;
	Material* material_;
	UploadBuffer materialBuffer_;
	Matrix4x4 matWorld_;
	UploadBuffer worldMatBuffer_;
	std::filesystem::path name_;
};