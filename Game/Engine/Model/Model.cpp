#include "Model.h"

#include <cassert>
#include <fstream>
#include <sstream>

#include <d3dx12.h>

#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Texture/TextureManager.h"

void Model::Create(const std::filesystem::path& modelPath) {
	LoadOBJFile(modelPath);
}

void Model::LoadOBJFile(const std::filesystem::path& modelPath) {
	
	std::vector<Vertex> vertexPos; //!< 構築するModelData
	std::vector<uint16_t> indices;
	std::vector<Vector3> positions; //!< 位置
	std::vector<Vector3> normals; //!< 法線
	std::vector<Vector2> texcoords; //!< テクスチャ座標
	std::string line; //!< ファイルから読み込んだ1行を格納するもの

	std::ifstream file(modelPath); //!< ファイルを開く
	assert(file.is_open()); //!< とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string indentifier;
		std::istringstream s(line);
		s >> indentifier; //!< 先頭の識別子を読む
		if (indentifier == "#") {
			continue;
		}
		// identifierに応じた処理
		else if (indentifier == "o") {
			meshes_.emplace_back(Mesh());
		}
		else if (indentifier == "v") {
			Vector3 position;
			s >> position.x >> position.y >> position.z;
			position.z *= -1.0f;
			positions.emplace_back(position);
		}
		else if (indentifier == "vt") {
			Vector2 texcood;
			s >> texcood.x >> texcood.y;
			texcood.y = 1.0f - texcood.y;
			texcoords.emplace_back(texcood);
		}
		else if (indentifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.z *= -1.0f;
			normals.emplace_back(normal);
		}
		else if (indentifier == "f") {
			Vertex triangle[3]{};
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinion;
				s >> vertexDefinion;
				// 頂点の要素へのIndexは「位置 / UV / 法線」で格納されているので、
				// 分解してIndexを取得する
				std::istringstream v(vertexDefinion);
				uint32_t elementIdices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); //!< /区切りでインデックスを読んでいく
					if (index.empty()) {
						// テクスチャ座標が省略されている場合、要素に0を代入
						elementIdices[element] = 0;
					}
					else {
						elementIdices[element] = std::stoi(index);
					}
					if (element == 0) {
						indices.emplace_back(elementIdices[element]);
					}
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector3 position = positions[elementIdices[0] - 1];
				Vector2 texcoord;
				// UV座標が無ければ{0.0f,0.0f}
				if (texcoords.empty()) {
					texcoord = { 0.0f,0.0f };
				}
				else {
					texcoord = texcoords[elementIdices[1] - 1];
				}
				Vector3 normal = normals[elementIdices[2] - 1];
				triangle[faceVertex] = { position,normal ,texcoord };
			}
			// 頂点を逆順することで、回り順を逆にする
			vertexPos.emplace_back(triangle[2]);
			vertexPos.emplace_back(triangle[1]);
			vertexPos.emplace_back(triangle[0]);
		}
		else if (indentifier == "usemtl") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialName;
			s >> materialName;
			//基本的にobjファイルと同一階層にmtlは存在させるので,ディレクトリ名とファイル名を渡す
			LoadMTLFile(modelPath);
			meshes_.back().indexCount = uint32_t(indices.size());
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

void Model::LoadMTLFile(const std::filesystem::path& modelPath) {
	// 1. 中で必要となる変数の宣言
	std::filesystem::path materialPath; //!< 構築するMaterialData
	std::string line; //!< ファイルから読み込んだ1行を格納するもの
	// 2. ファイルを開く
	std::ifstream file(modelPath);
	assert(file.is_open());
	// 3. 実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		// identifierに応じた処置
		if (identifier == "newmtl") {
			materials_.emplace_back(Material());
		}
		else if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialPath = modelPath.parent_path() / textureFilename;
			materials_.back().textureHandle = (TextureManager::GetInstance()->Load(materialPath));
			materials_.back().color = { 1.0f,1.0f,1.0f,1.0f };
		}
		else if (identifier == "Kd") {
			Vector4 color{};
			s >> color.x >> color.y >> color.z;
			color.w = 1.0f;
			materials_.back().color = color;
		}
	}
}
