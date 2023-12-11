#pragma once

#include <filesystem>

#include "DirectXTex.h"

#include "Engine/Graphics/GpuResource.h"
#include "Engine/Graphics/DescriptorHandle.h"

class Texture : public GpuResource {
public:
	void CreateFromWICFile(const std::filesystem::path& path);

	const D3D12_RESOURCE_DESC& GetDesc() const { return desc_; }
	const DescriptorHandle& GetSRV() const { return srvHandle_; }
	std::filesystem::path GetName() const { return name_.stem(); }
private:
	DirectX::ScratchImage LoadTexture(const std::filesystem::path& path);
	void CreateResource(const DirectX::TexMetadata& metadata);
	[[nodiscard]]
	void UploadTextureData(const DirectX::ScratchImage& mipImages);
	void CreateView(const DirectX::TexMetadata& metadata);
	D3D12_RESOURCE_DESC desc_{};
	DescriptorHandle srvHandle_;
	std::filesystem::path name_;
};
