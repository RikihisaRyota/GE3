#include "Sprite.h"

#include <cassert>
#include <fstream>
#include <sstream>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/WinApp/WinApp.h"

const Matrix4x4 Sprite::sMatProjection = MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kWindowWidth, (float)WinApp::kWindowHeight, 0.0f, 1.0f);

Sprite* Sprite::Create(TextureHandle  textureHandle, Vector2 position, Vector4 color,
	Vector2 anchorpoint, bool isFlipX, bool isFlipY) {
	// 仮サイズ
	Vector2 size = { 100.0f, 100.0f };

	{
		// テクスチャ情報取得
		const D3D12_RESOURCE_DESC& resDesc =
			TextureManager::GetInstance()->GetTexture(textureHandle).GetDesc();
		// スプライトのサイズをテクスチャのサイズに設定
		size = { (float)resDesc.Width, (float)resDesc.Height };
	}

	// Spriteのインスタンスを生成
	Sprite* sprite =
		new Sprite(textureHandle, position, size, color, anchorpoint, isFlipX, isFlipY);
	if (sprite == nullptr) {
		return nullptr;
	}

	// 初期化
	if (!sprite->Initialize()) {
		delete sprite;
		assert(0);
		return nullptr;
	}

	return sprite;
}

Sprite::Sprite() {}

Sprite::Sprite(TextureHandle textureHandle, Vector2 position, Vector2 size, Vector4 color, Vector2 anchorpoint,
	bool isFlipX, bool isFlipY) {
	position_ = position;
	size_ = size;
	anchorPoint_ = anchorpoint;
	matWorld_ = MakeIdentity4x4();
	color_ = color;
	textureHandle_ = textureHandle;
	isFlipX_ = isFlipX;
	isFlipY_ = isFlipY;
	texSize_ = size;
}

void Sprite::Draw(CommandContext& commandContext) {
	// ワールド行列の更新
	matWorld_ = MakeIdentity4x4();
	matWorld_ *= MakeRotateZMatrix(rotation_);
	matWorld_ *= MakeTranslateMatrix(Vector3(position_.x, position_.y, 0.0f));

	materialBuffer_.Copy(&color_, sizeof(Vector4));
	matWorld_ = matWorld_ * sMatProjection; // 行列の合成
	worldMatBuffer_.Copy(&matWorld_, sizeof(Matrix4x4));
	commandContext.SetVertexBuffer(0, vbView_);
	commandContext.SetIndexBuffer(ibView_);
	commandContext.SetGraphicsConstantBuffer(0, worldMatBuffer_.GetGPUVirtualAddress());
	commandContext.SetGraphicsConstantBuffer(1, materialBuffer_.GetGPUVirtualAddress());
	commandContext.SetGraphicsDescriptorTable(2, TextureManager::GetInstance()->GetTexture(textureHandle_).GetSRV());
	commandContext.SetGraphicsDescriptorTable(3, SamplerManager::LinearWrap);
	commandContext.DrawIndexed(6);
}

void Sprite::SetRotation(float rotation) {
	rotation_ = rotation;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

void Sprite::SetPosition(const Vector2& position) {
	position_ = position;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

void Sprite::SetSize(const Vector2& size) {
	size_ = size;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

void Sprite::SetAnchorPoint(const Vector2& anchorpoint) {
	anchorPoint_ = anchorpoint;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

void Sprite::SetIsFlipX(bool isFlipX) {
	isFlipX_ = isFlipX;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

void Sprite::SetIsFlipY(bool isFlipY) {
	isFlipY_ = isFlipY;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

void Sprite::SetTextureRect(const Vector2& texBase, const Vector2& texSize) {
	texBase_ = texBase;
	texSize_ = texSize;

	// 頂点バッファへのデータ転送
	TransferVertices();
}

bool Sprite::Initialize() {
	name_ = TextureManager::GetInstance()->GetTexture(textureHandle_).GetName();

	resourceDesc_ = TextureManager::GetInstance()->GetTexture(textureHandle_).GetDesc();
	// 頂点バッファ
	{
		struct Vertex {
			Vector3 position;
			Vector2 texcoord;
		};
		std::vector<Vertex> vertices = {
			// 前
			{ { -0.5f, -0.5f, +0.0f },{0.0f,1.0f} }, // 左下
			{ { -0.5f, +0.5f, +0.0f },{0.0f,0.0f} }, // 左上
			{ { +0.5f, -0.5f, +0.0f },{1.0f,1.0f} }, // 右下
			{ { +0.5f, +0.5f, +0.0f },{1.0f,0.0f} }, // 右上
		};

		size_t sizeIB = sizeof(Vertex) * vertices.size();

		vertexBuffer_.Create(L"SpriteVertexBuffer", sizeIB);

		vertexBuffer_.Copy(vertices.data(), sizeIB);

		vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
		vbView_.SizeInBytes = UINT(sizeIB);
		vbView_.StrideInBytes = sizeof(Vertex);
	}
	// インデックスバッファ
	{
		std::vector<uint16_t>indices = {
		0, 1, 3,
		2, 0, 3,
		};

		size_t sizeIB = sizeof(uint16_t) * indices.size();

		indexBuffer_.Create(L"SpriteIndexBuffer", sizeIB);

		indexBuffer_.Copy(indices.data(), sizeIB);

		// インデックスバッファビューの作成
		ibView_.BufferLocation = indexBuffer_.GetGPUVirtualAddress();
		ibView_.Format = DXGI_FORMAT_R16_UINT;
		ibView_.SizeInBytes = UINT(sizeIB);
	}
	material_ = new Material();
	material_->color = color_;
	materialBuffer_.Create(name_.wstring() + L"materialBuffer", sizeof(Material));
	materialBuffer_.Copy(material_, sizeof(Material));

	worldMatBuffer_.Create(name_.wstring() + L"worldMatBuffer", sizeof(Matrix4x4));
	worldMatBuffer_.Copy(&matWorld_, sizeof(Matrix4x4));

	TransferVertices();

	return true;
}

void Sprite::TransferVertices() {
	// 左下、左上、右下、右上
	enum { LB, LT, RB, RT };

	float left = (0.0f - anchorPoint_.x) * size_.x;
	float right = (1.0f - anchorPoint_.x) * size_.x;
	float top = (0.0f - anchorPoint_.y) * size_.y;
	float bottom = (1.0f - anchorPoint_.y) * size_.y;
	if (isFlipX_) { // 左右入れ替え
		left = -left;
		right = -right;
	}

	if (isFlipY_) { // 上下入れ替え
		top = -top;
		bottom = -bottom;
	}
	float tex_left = texBase_.x / resourceDesc_.Width;
	float tex_right = (texBase_.x + texSize_.x) / resourceDesc_.Width;
	float tex_top = texBase_.y / resourceDesc_.Height;
	float tex_bottom = (texBase_.y + texSize_.y) / resourceDesc_.Height;

	// 頂点データ
	std::vector<Vertex> vertices = {
		// 前
		{ { left,	bottom,	0.0f }	,{ tex_left,	tex_bottom } }, // 左下
		{ { left,	top,	0.0f }	,{ tex_left,	tex_top } }, // 左上
		{ { right,	bottom, 0.0f }	,{ tex_right,	tex_bottom }}, // 右下
		{ { right,	top,	0.0f }	,{ tex_right,	tex_top }}, // 右上
	};
	// 頂点バッファへのデータ転送
	vertexBuffer_.Copy(vertices.data(), sizeof(Vertex) * vertices.size());
}
