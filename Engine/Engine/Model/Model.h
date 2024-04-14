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

struct aiNode;
class Model {
private:
	struct Mesh {
		uint32_t indexCount;
		Vector3 min;
		Vector3 max;
	};

	struct Material {
		Vector4 color;
	};

	struct Vertex {
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;
	};
	struct Node {
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};
	struct MeshData {
		UploadBuffer indexBuffer{};
		D3D12_INDEX_BUFFER_VIEW ibView{};
		Mesh* meshes_{};
		std::vector<Vertex> vertices;
		Node rootNode;
	};
public:

	void Create(const std::filesystem::path& modelPath);
	const std::filesystem::path& GetName() const { return name_; }
	const std::vector<std::unique_ptr<MeshData>>& GetMeshData() const { return meshDatas_; }
	const D3D12_VERTEX_BUFFER_VIEW GetVBView()const { return vbView_; }
	const TextureHandle GetTextureHandle()const { return textureHandle_; }
	const UploadBuffer& GetMaterialBuffer()const { return materialBuffer_; }
	void SetMaterialColor(const Vector4& color);
private:
	void LoadFile(const std::filesystem::path& modelPath);
	Node ReadNode(aiNode* node);
	std::vector<std::unique_ptr<MeshData>> meshDatas_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	TextureHandle textureHandle_;
	Material* material_;
	UploadBuffer materialBuffer_;
	std::filesystem::path name_;
};