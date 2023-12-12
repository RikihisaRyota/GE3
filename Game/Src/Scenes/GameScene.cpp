#include "GameScene.h"

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"

#include "Engine/ImGui/ImGuiManager.h"

GameScene::GameScene() {
	viewProjection_.Initialize();
	gpuParticle_ = std::make_unique<GPUParticle>();
	debugCamera_ = std::make_unique<DebugCamera>();
}

void GameScene::Initialize() {
	gpuParticle_->Initialize();
}

void GameScene::Update() {
	/*ImGui::Begin("fps");
	ImGui::Text("Frame rate: %3.0f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Delta Time: %.4f", ImGui::GetIO().DeltaTime);
	ImGui::End();*/

	gpuParticle_->Update();
	debugCamera_->Update(&viewProjection_);
}

void GameScene::Draw(const CommandContext& commandContext) {
	gpuParticle_->Render(viewProjection_);
}
