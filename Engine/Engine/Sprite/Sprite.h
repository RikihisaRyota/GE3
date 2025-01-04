#pragma once
#include <filesystem>
#include <cstdint>
#include <vector>
#include <memory>

#include <d3d12.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"

#include "Engine/Math/Vector4.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Matrix4x4.h"

class CommandContext;
class Sprite {
private:
	struct Material {
		Vector4 color;
	};

	struct Vertex {
		Vector3 position;
		Vector2 texcoord;
	};

public:
	Sprite();
	Sprite(TextureHandle textureHandle, Vector2 position, Vector2 size, Vector4 color, Vector2 anchorpoint,
		bool isFlipX, bool isFlipY);
	// 生成
	static Sprite* Create(
		TextureHandle  textureHandle, Vector2 position, Vector4 color = { 1, 1, 1, 1 },
		Vector2 anchorpoint = { 0.0f, 0.0f }, bool isFlipX = false, bool isFlipY = false);
	// 描画
	void Draw(CommandContext& commandContext);

	const std::filesystem::path& GetName() const { return name_; }
	const TextureHandle& GetTextureHandle() const { return textureHandle_; }
	const UploadBuffer& GetMaterialBuffer()const { return materialBuffer_; }
	const UploadBuffer& GetWorldMatBuffer()const { return worldMatBuffer_; }
	/// <summary>
	/// 座標の設定
	/// </summary>
	/// <param name="position">座標</param>
	void SetPosition(const Vector2& position);

	const Vector2& GetPosition() { return position_; }

	/// <summary>
	/// 角度の設定
	/// </summary>
	/// <param name="rotation">角度</param>
	void SetRotation(float rotation);

	float GetRotation() { return rotation_; }

	/// <summary>
	/// サイズの設定
	/// </summary>
	/// <param name="size">サイズ</param>
	void SetSize(const Vector2& size);

	const Vector2& GetSize() { return size_; }

	/// <summary>
	/// アンカーポイントの設定
	/// </summary>
	/// <param name="anchorpoint">アンカーポイント</param>
	void SetAnchorPoint(const Vector2& anchorpoint);

	const Vector2& GetAnchorPoint() { return anchorPoint_; }

	/// <summary>
	/// 色の設定
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(const Vector4& color) { color_ = color; };

	const Vector4& GetColor() { return color_; }

	/// <summary>
	/// 左右反転の設定
	/// </summary>
	/// <param name="isFlipX">左右反転</param>
	void SetIsFlipX(bool isFlipX);

	bool GetIsFlipX() { return isFlipX_; }

	/// <summary>
	/// 上下反転の設定
	/// </summary>
	/// <param name="isFlipX">上下反転</param>
	void SetIsFlipY(bool isFlipY);

	bool GetIsFlipY() { return isFlipY_; }

	/// <summary>
	/// テクスチャ範囲設定
	/// </summary>
	/// <param name="texBase">テクスチャ左上座標</param>
	/// <param name="texSize">テクスチャサイズ</param>
	void SetTextureRect(const Vector2& texBase, const Vector2& texSize);
private:
	static const Matrix4x4 sMatProjection;
	bool Initialize();
	void TransferVertices();


	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	TextureHandle textureHandle_;
	Material* material_;
	UploadBuffer materialBuffer_;
	Matrix4x4 matWorld_;
	UploadBuffer worldMatBuffer_;
	std::filesystem::path name_;

	// Z軸回りの回転角
	float rotation_ = 0.0f;
	// 座標
	Vector2 position_{};
	// スプライト幅、高さ
	Vector2 size_ = { 100.0f, 100.0f };
	// アンカーポイント
	Vector2 anchorPoint_ = { 0, 0 };
	// 色
	Vector4 color_ = { 1, 1, 1, 1 };
	// 左右反転
	bool isFlipX_ = false;
	// 上下反転
	bool isFlipY_ = false;
	// テクスチャ始点
	Vector2 texBase_ = { 0, 0 };
	// テクスチャ幅、高さ
	Vector2 texSize_ = { 100.0f, 100.0f };
	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc_;
};