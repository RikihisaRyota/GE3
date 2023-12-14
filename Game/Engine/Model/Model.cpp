#include "Model.h"

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

void Model::Create(const std::filesystem::path& modelPath) {
	LoadFile(modelPath);
}

void Model::LoadFile(const std::filesystem::path& modelPath) {
	name_ = modelPath.stem();

	std::string path = modelPath.string() + "/" + modelPath.stem().string() + ".obj";

	std::vector<Vertex> vertexPos; //!< 構築するModelData
	std::vector<Index> indices;

	Assimp::Importer importer{};

	const aiScene* scene = importer.ReadFile(path.c_str(),
		aiProcess_Triangulate |
		aiProcess_FlipUVs);
	assert(scene->HasMeshes()); // メッシュがないものは対応しない
	// メッシュ解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは今回は未対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回は未対応

		// 頂点データを解析
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			Vertex vertex{};
			vertex.position = { position.x,position.y,position.z };
			vertex.normal = { normal.x,normal.y,normal.z };
			vertex.texcoord = { texcoord.x,texcoord.y };
			vertex.position.x *= -1.0f;
			vertex.normal.x *= -1.0f;

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

		meshDatas_.emplace_back(std::make_unique<MeshData>());
		auto& currentModelData = meshDatas_.back();
		currentModelData->meshes_ = new Mesh();
		currentModelData->meshes_->indexCount = uint32_t(indices.size());

		currentModelData->indexBuffer_.Create(modelPath.stem().wstring() + L"IndexBuffer", indices.size() * sizeof(indices[0]));
		currentModelData->indexBuffer_.Copy(indices.data(), indices.size() * sizeof(indices[0]));
		currentModelData->ibView_.BufferLocation = currentModelData->indexBuffer_.GetGPUVirtualAddress();
		currentModelData->ibView_.SizeInBytes = UINT(currentModelData->indexBuffer_.GetBufferSize());
		currentModelData->ibView_.Format = DXGI_FORMAT_R32_UINT;
	}
	vertexBuffer_.Create(modelPath.stem().wstring() + L"VertexBuffer", vertexPos.size() * sizeof(vertexPos[0]));
	vertexBuffer_.Copy(vertexPos.data(), vertexPos.size() * sizeof(vertexPos[0]));
	vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
	vbView_.SizeInBytes = UINT(vertexBuffer_.GetBufferSize());
	vbView_.StrideInBytes = sizeof(vertexPos[0]);
	// Material解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		// 最後に見つかったMaterialデータを利用
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			textureHandle = (TextureManager::GetInstance()->Load(modelPath / textureFilePath.C_Str()));
			material_ = new Material();
			material_->color = { 1.0f,1.0f,1.0f,1.0f };
			materialBuffer_.Create(modelPath.stem().wstring() + L"materialBuffer", sizeof(Material));
			materialBuffer_.Copy(material_, sizeof(Material));
		}
	}
}