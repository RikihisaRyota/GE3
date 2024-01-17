#include "Texture.h"

#include <vector>

#include <d3dx12.h>


#include "Engine/ConvertString/ConvertString.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/RenderManager.h"

void Texture::CreateFromWICFile(const std::filesystem::path& path) {
	// TextureデータをCPUにロード
	DirectX::ScratchImage mipImage = LoadTexture(path);

	const DirectX::TexMetadata& metadata = mipImage.GetMetadata();

	CreateResource(metadata,path);

	UploadTextureData(mipImage);

	CreateView(metadata);
}

DirectX::ScratchImage Texture::LoadTexture(const std::filesystem::path& path) {
	// テクスチャを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(path.generic_string());
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// 元のテクスチャが1x1の場合はミップマップの生成は不要なのでそのまま返す
	if (image.GetMetadata().width == 1 && image.GetMetadata().height == 1) {
		return image;
	}

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	return mipImages;
}

void Texture::CreateResource(const DirectX::TexMetadata& metadata, const std::filesystem::path& path) {
	
	desc_.Width = UINT(metadata.width);// Textureの幅
	desc_.Height = UINT(metadata.height);// Textureの高さ
	desc_.MipLevels = UINT16(metadata.mipLevels);// mipmapの数
	desc_.DepthOrArraySize = UINT16(metadata.arraySize);// 奥行き or 配列textureの配列数
	desc_.Format = metadata.format;// TextureのFormat
	desc_.SampleDesc.Count = 1;// サンプリングカウント。1固定
	desc_.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);// Textureの次元数。普段使っているのは2次元
	D3D12_HEAP_PROPERTIES heapPropeteies = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = GraphicsCore::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapPropeteies,
		D3D12_HEAP_FLAG_NONE,
		&desc_,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(resource_.GetAddressOf()));
	resource_->SetName(path.c_str());
	state_ = D3D12_RESOURCE_STATE_COMMON;
}

void Texture::UploadTextureData(const DirectX::ScratchImage& mipImages) {
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto graphicsCore = GraphicsCore::GetInstance();
	auto& commandQueue = graphicsCore->GetCommandQueue();
	CommandContext commandContext;
	commandContext.Create();
	std::vector<D3D12_SUBRESOURCE_DATA> subResources{};
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subResources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(resource_.Get(), 0, UINT(subResources.size()));
	D3D12_RESOURCE_DESC intermediateDesc = CD3DX12_RESOURCE_DESC::Buffer(intermediateSize);
	D3D12_HEAP_PROPERTIES heapPropeteies = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	GpuResource intermediateResource{};
	device->CreateCommittedResource(
		&heapPropeteies,
		D3D12_HEAP_FLAG_NONE,
		&intermediateDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(intermediateResource.GetAddressOf())
	);
	intermediateResource.SetState(D3D12_RESOURCE_STATE_GENERIC_READ);
	intermediateResource->SetName(L"intermediateResource");
	commandContext.TransitionResource(*this, D3D12_RESOURCE_STATE_COPY_DEST);
	commandContext.FlushResourceBarriers();

	UpdateSubresources(commandContext,resource_.Get(), intermediateResource.GetResource(), 0, 0, UINT(subResources.size()), subResources.data());

	commandContext.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandContext.Close();
	commandQueue.Execute(commandContext);
	commandQueue.Signal();
	commandQueue.WaitForGPU();

}

void Texture::CreateView(const DirectX::TexMetadata& metadata) {
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto graphics = GraphicsCore::GetInstance();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(resource_.Get(), &srvDesc,srvHandle_);
}
