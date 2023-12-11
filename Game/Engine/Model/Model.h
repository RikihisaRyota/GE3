#pragma once
#include <filesystem>
#include <cstdint>

#include <d3d12.h>

#include "../../Engine/Graphics/UploadBuffer.h"

class Model {
public:
	Model* Create(const std::filesystem::path& modelPath);
private:
	struct Mesh {
		uint32_t indexCount;
	};

	struct Material {

	};

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};


};