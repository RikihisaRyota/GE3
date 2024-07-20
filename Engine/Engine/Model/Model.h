#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <map>
#include <memory>
#include <optional>

#include <d3d12.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/DescriptorHandle.h"

#include "Engine/Math/Vector4.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Matrix4x4.h"
#include "Engine/Math/Quaternion.h"

struct aiNode;
class Model {
public:
	struct Mesh {
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		Vector3 min;
		Vector3 max;
	};

	struct Material {
		Vector4 color;
		float environmentCoefficient;
	};

	struct Vertex {
		Vector4 position;
		Vector3 normal;
		Vector2 texcoord;
	};

	struct EulerTransform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	struct QuaternionTransform {
		Vector3 scale;
		Quaternion rotate;
		Vector3 translate;
	};

	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};

	struct JointWeightData {
		Matrix4x4 inverseBindPoseMatrix;
		std::vector<VertexWeightData> vertexWeights;
	};

	struct MeshData {
		std::map<std::string, JointWeightData> skinClusterData;

		Mesh* meshes{};
		std::vector<Vertex> vertices;
		Node rootNode;
	};

public:

	void Create(const std::filesystem::path& modelPath);
	const std::filesystem::path& GetName() const { return name_; }
	const std::vector<std::unique_ptr<MeshData>>& GetMeshData() const { return meshData_; }
	const TextureHandle GetTextureHandle()const { return textureHandle_; }
	const UploadBuffer& GetMaterialBuffer()const { return materialBuffer_; }
	Material& GetMaterialData() { return *material_; }
	const std::filesystem::path& GetPath()const { return path_; }
	void SetMaterialColor(const Vector4& color);
	Vector4& GetMaterialColor() { return material_->color; }
	const D3D12_INDEX_BUFFER_VIEW& GetIndexView() const { return ibView; }
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexView() const { return vbView; }
	UploadBuffer& GetIndexBuffer() { return indexBuffer; }
	UploadBuffer& GetVertexBuffer() { return vertexBuffer; }
	const DescriptorHandle& GetVertexSRV() const { return vertexSRV; }
	const DescriptorHandle& GetIndexSRV() const { return indexSRV; }
	const uint32_t GetAllVertexCount()const;
	const uint32_t GetAllIndexCount()const;
private:
	void LoadFile(const std::filesystem::path& modelPath);
	Node ReadNode(aiNode* node);
	std::vector<std::unique_ptr<MeshData>> meshData_;
	TextureHandle textureHandle_;
	Material* material_;
	UploadBuffer materialBuffer_;
	std::filesystem::path name_;
	std::filesystem::path path_;



	UploadBuffer indexBuffer{};
	D3D12_INDEX_BUFFER_VIEW ibView{};

	UploadBuffer vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	DescriptorHandle vertexSRV{};
	DescriptorHandle indexSRV{};
};