#include "Transition.h"

#include "Engine/Sprite/SpriteManager.h"
#include "Engine/Texture/TextureManager.h"

const float Transition::kMaxTime = 60.0f;

Transition::Transition() {
	spriteHandle_ = SpriteManager::GetInstance()->Create(TextureManager::GetInstance()->Load("Resources/Images/white1x1.png"), { 1280.0f * 0.5f, 720.0f * 0.5f }, { 0.5f,0.5f } ,{1.0f,1.0f,1.0f,0.0f});
	SpriteManager::GetInstance()->GetSprite(spriteHandle_)->SetSize({ 1280.0f,720.0f });
	time_ = 0.0f;
	isSceneStart_ = true;
	isSceneChange_ = false;
	isTransition_ = false;
}

void Transition::Initialize() {
	time_ = 0.0f;
	isSceneChange_ = false;
	isTransition_ = true;
}

void Transition::Update() {
	time_+=1.0f;
	if (!isSceneChange_) {
		SpriteManager::GetInstance()->GetSprite(spriteHandle_)->SetColor({ 1.0f,1.0f,1.0, time_ / kMaxTime });
		if (time_ >= kMaxTime) {
			time_ = 0.0f;
			isSceneChange_ = true;
		}
	}
	else {
		SpriteManager::GetInstance()->GetSprite(spriteHandle_)->SetColor({ 1.0f,1.0f,1.0,1.0f - (time_ / kMaxTime)});
		if (time_ >= kMaxTime) {
			time_ = 0.0f;
			isTransition_ = false;
			isSceneStart_ = true;
		}
	}
}

void Transition::Draw(CommandContext& commandContext) {
	SpriteManager::GetInstance()->Draw(spriteHandle_, commandContext);
}
