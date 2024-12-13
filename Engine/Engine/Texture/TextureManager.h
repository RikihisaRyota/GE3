#pragma once
/**
 * @file TextureManager.h
 * @brief Textureの管理
 */
#include <filesystem>
#include <string>
#include <memory>
#include <vector>

#include "TextureHandle.h"
#include "Texture.h"

class TextureManager {
public:
	static TextureManager* GetInstance();
	// texture読み込み
	TextureHandle Load(const std::filesystem::path path);

	Texture& GetTexture(const TextureHandle& textureHandle) { return *textures_[textureHandle.index_]; }
	Texture& GetTexture(uint32_t textureHandle) { return *textures_[textureHandle]; }
	
	// descriptorIndexからtextures_のどこに入ってるか知りたい
	int32_t GetTextureLocation(uint32_t descriptorIndex);

	size_t GetTextureSize() { return textures_.size(); }
private:
	std::vector<std::unique_ptr<Texture>> textures_;
};