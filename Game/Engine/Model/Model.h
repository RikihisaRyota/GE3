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

class Model {
private:
	struct Mesh {
		uint32_t indexCount;
	};

	struct Material {
		Vector4 color;
	};

	struct Vertex {
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;
	};

	struct ModelData {
		UploadBuffer vertexBuffer_;
		D3D12_VERTEX_BUFFER_VIEW vbView_{};
		UploadBuffer indexBuffer_;
		D3D12_INDEX_BUFFER_VIEW ibView_{};
		TextureHandle textureHandle;
		Mesh* meshes_;
		Material* material_;
		UploadBuffer materialBuffer_;
	};

public:
	void Create(const std::filesystem::path& modelPath);
	const std::filesystem::path& GetName() const { return name_; }
	const std::vector<std::unique_ptr<ModelData>>& GetModelData() const {return modelDatas_;}
private:
	void LoadFile(const std::filesystem::path& modelPath);
	std::vector<std::unique_ptr<ModelData>> modelDatas_;
	std::filesystem::path name_;
};