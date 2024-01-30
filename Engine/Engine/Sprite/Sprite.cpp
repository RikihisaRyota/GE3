#include "Sprite.h"

#include <cassert>
#include <fstream>
#include <sstream>

#include "Engine/Texture/TextureManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/WinApp/WinApp.h"

const Matrix4x4 Sprite::sMatProjection = MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kWindowWidth, (float)WinApp::kWindowWidth, 0.0f, 1.0f);

void Sprite::Create(const std::filesystem::path& texturePath) {
	LoadFile(texturePath);
}

void Sprite::SetPosition(const Vector2 pos) {
	matWorld_ = MakeIdentity4x4();
	matWorld_ *= MakeTranslateMatrix(Vector3(pos.x, pos.y, 0.0f));
	matWorld_ *= sMatProjection;
	worldMatBuffer_.Copy(&matWorld_, sizeof(Matrix4x4));
}

void Sprite::LoadFile(const std::filesystem::path& texturePath) {
	name_ = texturePath.stem();

	textureHandle_ = (TextureManager::GetInstance()->Load(texturePath));
	material_ = new Material();
	material_->color = { 1.0f,1.0f,1.0f,1.0f };
	materialBuffer_.Create(texturePath.stem().wstring() + L"materialBuffer", sizeof(Material));
	materialBuffer_.Copy(material_, sizeof(Material));

	worldMatBuffer_.Create(texturePath.stem().wstring() + L"worldMatBuffer", sizeof(Matrix4x4));
	worldMatBuffer_.Copy(&matWorld_, sizeof(Matrix4x4));
}
