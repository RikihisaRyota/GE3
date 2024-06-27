#include "TextureManager.h"

TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}

TextureHandle TextureManager::Load(const std::filesystem::path path) {
	TextureHandle handle;
	// 読み込み済みか探す
	auto iter = std::find_if(textures_.begin(), textures_.end(), [&](const auto& texture) { return texture->GetName() == path.stem(); });
	// 読み込み済み
	if (iter != textures_.end()) {
		handle.index_ = std::distance(textures_.begin(), iter);
		return handle;
	}

	// 最後尾に読み込む
	handle.index_ = textures_.size();

	auto texture = std::make_unique<Texture>();
	texture->CreateFromWICFile(path);

	textures_.emplace_back(std::move(texture));
	return handle;
}

uint32_t TextureManager::GetTextureLocation(uint32_t descriptorIndex) {
	// TODO: return ステートメントをここに挿入します
	for (uint32_t i = 0; auto & texture : textures_) {
		if (descriptorIndex == texture->GetDescriptorIndex()) {
			return i;
		}
		i++;
	}
	return -1;
}
