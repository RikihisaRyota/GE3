#include "GameScene.h"

#include "Engine/Audio/Audio.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"

#include "Engine/Sprite/SpriteManager.h"

GameScene::GameScene() {
	debugCamera_ = std::make_unique<DebugCamera>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	player_ = std::make_unique<Player>();
	followCamera_ = std::make_unique<FollowCamera>();

	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball");
	terrainHandle_ = ModelManager::GetInstance()->Load("Resources/Models/terrain");

	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	/*soundHandle_ = Audio::GetInstance()->SoundLoadWave("play.wav");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);*/

}

GameScene::~GameScene() {}

void GameScene::Initialize() {
	//Audio::GetInstance()->SoundPlayLoopStart(playHandle_);
	viewProjection_.Initialize();
	worldTransform_.Initialize();

	player_->SetViewProjection(&viewProjection_);
	player_->Initialize();

	followCamera_->SetTarget(&player_->GetWorldTransform());
	followCamera_->SetViewProjection(&viewProjection_);

	gpuParticleManager_->Initialize();
	/*{
		EmitterForGPU emitterForGPU = {
		.min = {-0.5f,-1.0f,-0.5f},
		.maxParticleNum = 1 << 24,
		.max = {0.5f,1.0f,0.5f},
		.createParticleNum = 1 << 10,
		.position = {0.0,0.0f,0.0f},
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}*/
	{
		EmitterForGPU emitterForGPU = {
		.min = {-10.0f,-15.0f,-10.0f},
		.maxParticleNum = 1 << 24,
		.max = {10.0f,15.0f,30.0f},
		.createParticleNum = 1 << 10,
		.position = {-20.0,0.0f,0.0f},
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	{
		EmitterForGPU emitterForGPU = {
		.min = {-10.0f,-15.0f,-10.0f},
		.maxParticleNum = 1 << 24,
		.max = {10.0f,15.0f,30.0f},
		.createParticleNum = 1 << 10,
		.position = {20.0,0.0f,0.0f},
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	{
		EmitterForGPU emitterForGPU = {
		.min = {-15.0f,-10.0f,-10.0f},
		.maxParticleNum = 1 << 24,
		.max = {15.0f,10.0f,30.0f},
		.createParticleNum = 1 << 10,
		.position = {0.0f,20.0f,0.0f},
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	{
		EmitterForGPU emitterForGPU = {
		.min = {-15.0f,-10.0f,-10.0f},
		.maxParticleNum = 1 << 24,
		.max = {15.0f,10.0f,30.0f},
		.createParticleNum = 1 << 10,
		.position = {0.0f,-20.0f,-10.0f},
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	/*{
		EmitterForGPU emitterForGPU = {
		.min = {-10.0f,-10.0f,10.0f},
		.maxParticleNum = 1 << 20,
		.max = {10.0f,10.0f,30.0f},
		.createParticleNum = 1 << 10,
		.position = {0.0f,0.0f,10.0f},
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}*/
}

void GameScene::Update() {

#ifdef _DEBUG
	ImGui::Begin("fps");
	ImGui::Text("Frame rate: %3.0f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Delta Time: %.4f", ImGui::GetIO().DeltaTime);
	ImGui::End();
	ImGui::Begin("Sphere");
	ImGui::DragFloat3("scale", &worldTransform_.scale_.x, 0.1f, 0.0f);
	ImGui::End();
#endif // _DEBUG
	gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());
	//debugCamera_->Update(&viewProjection_);

	followCamera_->Update();

	player_->Update();

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);
}

void GameScene::Draw(CommandContext& commandContext) {
	gpuParticleManager_->Draw(viewProjection_, commandContext);

	player_->Draw(viewProjection_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection_, modelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection_, terrainHandle_, commandContext);
}

void GameScene::Finalize() {}
