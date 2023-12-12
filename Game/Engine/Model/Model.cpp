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
	std::string path = modelPath.string() + "/" + modelPath.stem().string() + ".obj";

	std::vector<Vertex> vertexPos; //!< 構築するModelData
	std::vector<uint16_t> indices;

	Assimp::Importer importer{};

	const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); // メッシュがないものは対応しない
	// メッシュ解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは今回は未対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回は未対応
		// Face解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート(四角形以上も対応できるようになる,CG3 04_00 assimpの資料)
			// ここからFaceの中身(Vertex)の解析
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint16_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				Vertex vertex{};
				vertex.position = { position.x,position.y,position.z };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };
				// aiProcess_MakeLeftHandedはz*=-1で、右手から左手に変換するので手動で対処
				vertex.position.z *= -1.0f;
				vertex.normal.z *= -1.0f;

				indices.push_back(vertexIndex);
				vertexPos.emplace_back(vertex);
			}
		}
		meshes_.emplace_back();
		meshes_.back().indexCount = uint32_t(indices.size());
	}

	// Material解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		// 最後に見つかったmaterialデータを利用
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			material_.textureHandle = (TextureManager::GetInstance()->Load(modelPath / textureFilePath.C_Str()));
			material_.color = { 1.0f,1.0f,1.0f,1.0f };
		}
	}

	vertexBuffer_.Create(L"Model VertexBuffer", vertexPos.size() * sizeof(vertexPos[0]));
	vertexBuffer_.Copy(vertexPos.data(), vertexPos.size() * sizeof(vertexPos[0]));
	vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
	vbView_.SizeInBytes = UINT(vertexBuffer_.GetBufferSize());
	vbView_.StrideInBytes = sizeof(vertexPos[0]);

	indexBuffer_.Create(L"Model IndexBuffer", indices.size() * sizeof(indices[0]));
	indexBuffer_.Copy(indices.data(), indices.size() * sizeof(indices[0]));
	ibView_.BufferLocation = indexBuffer_.GetGPUVirtualAddress();
	ibView_.SizeInBytes = UINT(indexBuffer_.GetBufferSize());
	ibView_.Format = DXGI_FORMAT_R16_UINT;
}