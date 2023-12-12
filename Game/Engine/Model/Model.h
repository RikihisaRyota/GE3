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
		TextureHandle textureHandle;
		Vector4 color;
	};

	struct Vertex {
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;
	};
public:
	void Create(const std::filesystem::path& modelPath);
	std::filesystem::path GetName() const { return name_.stem(); }
	const D3D12_VERTEX_BUFFER_VIEW GetVBView() const { return vbView_; }
	const D3D12_INDEX_BUFFER_VIEW GetIBView() const { return ibView_; }
	const std::vector<Mesh> GetMesh()const { return meshes_; }
	const Material GetMaterial() const { return material_; }
private:
	void LoadFile(const std::filesystem::path& modelPath);
	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	std::vector<Mesh> meshes_;
	Material material_;
	std::filesystem::path name_;
};