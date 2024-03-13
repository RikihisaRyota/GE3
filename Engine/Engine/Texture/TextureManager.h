#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "TextureHandle.h"
#include "Texture.h"

class TextureManager {
public:
	static TextureManager* GetInstance();

	TextureHandle Load(const std::filesystem::path path);

	Texture& GetTexture(const TextureHandle& textureHandle) { return *textures_[textureHandle.index_]; }
	Texture& GetTexture(uint32_t textureHandle) { return *textures_[textureHandle]; }
	size_t GetTextureSize() { return textures_.size(); }
private:
	std::vector<std::unique_ptr<Texture>> textures_;
};