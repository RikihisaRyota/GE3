#include "PlayerUI.h"

#include "Engine/Sprite/SpriteManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Input/Input.h"
#include "Engine/WinApp/WinApp.h"

PlayerUI::PlayerUI() {
	auto spriteManager = SpriteManager::GetInstance();
	auto textureManager = TextureManager::GetInstance();
	handle_.emplace_back(spriteManager->Create(textureManager->Load("Resources/Images/Player/playerReticle.png"), { float(WinApp::kWindowWidth) * 0.5f,float(WinApp::kWindowHeight) * 0.5f }, { 0.5f,0.5f }));
}

void PlayerUI::Initialize() {
	auto sprite = SpriteManager::GetInstance()->GetSprite(handle_.at(kReticle));

	sprite->SetColor({ 1.0f,0.5f,0.0f,1.0f });
	sprite->SetSize(sprite->GetSize() * 0.5f);
}

void PlayerUI::Update() {}

void PlayerUI::Draw(CommandContext& commandContext) {
	for (size_t i = 0; i < handle_.size(); ++i) {
		if (i == kReticle) {
			if (Input::GetInstance()->PushKey(DIK_LSHIFT)|| Input::GetInstance()->PushGamepadButton(Button::LT)) {
				SpriteManager::GetInstance()->Draw(handle_.at(i), commandContext);
			}
		}
		else {
			SpriteManager::GetInstance()->Draw(handle_.at(i), commandContext);
		}

	}

}
