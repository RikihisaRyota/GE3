#include "PlayerUI.h"

#include "Engine/Sprite/SpriteManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Input/Input.h"
#include "Engine/WinApp/WinApp.h"

PlayerUI::PlayerUI() {
	auto spriteManager = SpriteManager::GetInstance();
	auto textureManager = TextureManager::GetInstance();
	reticleUIHandle_ = spriteManager->Create(textureManager->Load("Resources/Images/Player/playerReticle.png"), { float(WinApp::kWindowWidth) * 0.5f,float(WinApp::kWindowHeight) * 0.5f }, { 0.5f,0.5f });
}

void PlayerUI::Initialize() {
	auto sprite = SpriteManager::GetInstance()->GetSprite(reticleUIHandle_);

	sprite->SetColor({ 1.0f,0.5f,0.0f,1.0f });
	sprite->SetSize(sprite->GetSize()*0.5f);
}

void PlayerUI::Update() {}

void PlayerUI::Draw(CommandContext& commandContext) {
	if (Input::GetInstance()->PushKey(DIK_LSHIFT)) {
		SpriteManager::GetInstance()->Draw(reticleUIHandle_, commandContext);
	}
}
