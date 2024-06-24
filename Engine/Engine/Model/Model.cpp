#include "Model.h"

#include <iostream>

#include <cassert>
#include <fstream>
#include <sstream>

#include <d3dx12.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Texture/TextureManager.h"

#include "Engine/Math/MyMath.h"
#include "Engine/Graphics/GraphicsCore.h"

void Model::Create(const std::filesystem::path& modelPath) {
	LoadFile(modelPath);
}

void Model::SetMaterialColor(const Vector4& color) {
	material_->color = color;
	materialBuffer_.Copy(material_, sizeof(Material));
}

void Model::LoadFile(const std::filesystem::path& modelPath) {
	auto device = GraphicsCore::GetInstance()->GetDevice();

	path_ = modelPath;
	name_ = modelPath.stem();

	std::vector<Vertex> vertexPos; //!< 構築するModelData
	std::vector<uint32_t> indices;

	Assimp::Importer importer{};

	const aiScene* scene = importer.ReadFile(modelPath.string(),
		aiProcess_Triangulate |
		aiProcess_FlipUVs);
	if (!scene) {
		std::cerr << "Error: Failed to load input file: " << importer.GetErrorString() << std::endl;
		assert(0);
	}
	assert(scene->HasMeshes()); // メッシュがないものは対応しない


	Vector3 minIndex{ FLT_MAX ,FLT_MAX ,FLT_MAX }, maxIndex{ FLT_MIN ,FLT_MIN ,FLT_MIN };

	// メッシュ解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		meshData_.emplace_back(std::make_unique<MeshData>());
		auto& currentModelData = meshData_.back();

		aiMesh* mesh = scene->mMeshes[meshIndex];

		assert(mesh->HasNormals()); // 法線がないMeshは今回は未対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回は未対応

		// 頂点データを解析
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			Vertex vertex{};
			vertex.position = { position.x,position.y,position.z,1.0f };
			vertex.normal = { normal.x,normal.y,normal.z };
			vertex.texcoord = { texcoord.x,texcoord.y };
			vertex.position.x *= -1.0f;
			vertex.normal.x *= -1.0f;
			minIndex.x = (std::min)(minIndex.x, position.x);
			minIndex.y = (std::min)(minIndex.y, position.y);
			minIndex.z = (std::min)(minIndex.z, position.z);
			maxIndex.x = (std::max)(maxIndex.x, position.x);
			maxIndex.y = (std::max)(maxIndex.y, position.y);
			maxIndex.z = (std::max)(maxIndex.z, position.z);
			vertexPos.emplace_back(vertex);
		}
		// インデックスデータを解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート(四角形以上も対応できるようになる,CG3 04_00 assimpの資料)
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[2]);
			indices.push_back(face.mIndices[1]);
		}
		// ボーン取得
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = currentModelData->skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale{}, translate{};
			aiQuaternion rotate{};
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MakeAffineMatrix(
				{ scale.x,scale.y,scale.z },
				{ rotate.x,-rotate.y,-rotate.z,rotate.w },
				{ -translate.x,translate.y,translate.z }
			);
			jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);
			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight,bone->mWeights[weightIndex].mVertexId });
			}
		}

		currentModelData->rootNode = ReadNode(scene->mRootNode);
		currentModelData->vertices = vertexPos;
		currentModelData->meshes = new Mesh();
		currentModelData->meshes->indexCount = uint32_t(indices.size());
		currentModelData->meshes->min = minIndex;
		currentModelData->meshes->max = maxIndex;
		currentModelData->indexBuffer.Create(name_.wstring() + L"IndexBuffer", indices.size() * sizeof(indices[0]));
		currentModelData->indexBuffer.Copy(indices.data(), indices.size() * sizeof(indices[0]));
		currentModelData->indexCountBuffer.Create(name_.wstring() + L"IndexCountBuffer",sizeof(UINT));
		size_t indexCount = vertexPos.size();
		currentModelData->indexCountBuffer.Copy(&indexCount,sizeof(UINT));
		currentModelData->ibView.BufferLocation = currentModelData->indexBuffer.GetGPUVirtualAddress();
		currentModelData->ibView.SizeInBytes = UINT(currentModelData->indexBuffer.GetBufferSize());
		currentModelData->ibView.Format = DXGI_FORMAT_R32_UINT;


		currentModelData->vertexBuffer.Create(name_.wstring() + L"VertexBuffer", vertexPos.size() * sizeof(vertexPos[0]));
		currentModelData->vertexBuffer.Copy(vertexPos.data(), vertexPos.size() * sizeof(vertexPos[0]));
		currentModelData->vertexCountBuffer.Create(name_.wstring() + L"VertexCountBuffer", sizeof(UINT));
		size_t vertexCount = vertexPos.size();
		currentModelData->vertexCountBuffer.Copy(&vertexCount, sizeof(UINT));
		currentModelData->vbView.BufferLocation = currentModelData->vertexBuffer.GetGPUVirtualAddress();
		currentModelData->vbView.SizeInBytes = UINT(currentModelData->vertexBuffer.GetBufferSize());
		currentModelData->vbView.StrideInBytes = sizeof(vertexPos[0]);

		currentModelData->srView = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC vertexSrvDesc{};
		vertexSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
		vertexSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		vertexSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		vertexSrvDesc.Buffer.FirstElement = 0;
		vertexSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		vertexSrvDesc.Buffer.NumElements = UINT(vertexPos.size());
		vertexSrvDesc.Buffer.StructureByteStride = sizeof(Model::Vertex);
		device->CreateShaderResourceView(currentModelData->vertexBuffer, &vertexSrvDesc, currentModelData->srView);

	}
	// Material解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		// 最後に見つかったMaterialデータを利用
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			textureHandle_ = (TextureManager::GetInstance()->Load((modelPath.parent_path().string() + "/" + textureFilePath.C_Str())));
			material_ = new Material();
			material_->color = { 1.0f,1.0f,1.0f,1.0f };
			materialBuffer_.Create(modelPath.stem().wstring() + L"materialBuffer", sizeof(Material));
			materialBuffer_.Copy(material_, sizeof(Material));
		}
	}
	if (!textureHandle_.IsValid()) {
		textureHandle_ = TextureManager::GetInstance()->Load("Resources/Images/white1x1.png");
		material_ = new Material();
		material_->color = { 1.0f,1.0f,1.0f,1.0f };
		materialBuffer_.Create(modelPath.stem().wstring() + L"materialBuffer", sizeof(Material));
		materialBuffer_.Copy(material_, sizeof(Material));
	}
}

Model::Node Model::ReadNode(aiNode* node) {
	Node result{};

	aiVector3D scale{}, translate{};
	aiQuaternion rotate{};
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	result.transform.translate = { -translate.x,translate.y,translate.z };
	result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}
