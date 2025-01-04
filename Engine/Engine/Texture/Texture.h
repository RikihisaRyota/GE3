#pragma once
/**
 * @file Texture.h
 * @brief texture
 */
#include <filesystem>

#include <d3d12.h>
#include "DirectXTex.h"

#include "Engine/Graphics/GpuResource.h"
#include "Engine/Graphics/DescriptorHandle.h"

class Texture : public GpuResource {
public:
	// dds読み込み
	void CreateFromWICFile(const std::filesystem::path& path);

	// Getter
	const D3D12_RESOURCE_DESC& GetDesc() const { return desc_; }
	const DescriptorHandle& GetSRV() const { return srvHandle_; }
	const std::filesystem::path GetName() const { return name_.stem(); }
	const std::filesystem::path GetPath() const { return name_; }
	const uint32_t GetDescriptorIndex() const { return descriptorIndex_; }
private:
	// Load
	DirectX::ScratchImage LoadTexture(const std::filesystem::path& path);
	// CreateResource
	void CreateResource(const DirectX::TexMetadata& metadata, const std::filesystem::path& path);
	[[nodiscard]]
	void UploadTextureData(const DirectX::ScratchImage& mipImages);
	void CreateView(const DirectX::TexMetadata& metadata);
	D3D12_RESOURCE_DESC desc_{};
	DescriptorHandle srvHandle_;
	uint32_t descriptorIndex_;
	std::filesystem::path name_;
};
