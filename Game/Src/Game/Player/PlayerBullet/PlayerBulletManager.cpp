#include "PlayerBulletManager.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/Json/JsonUtils.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/MyMath.h"
#include "Engine/WinApp/WinApp.h"
#include "Engine/Math/WorldTransform.h"
#include "../PlayerUI/PlayerUI.h"
#include "Engine/Sprite/SpriteManager.h"
#include "Engine/ImGui/ImGuiManager.h"

#include "Engine/Collision/CollisionAttribute.h"

#include "Src/Game/Boss/Boss.h"

PlayerBulletManager::PlayerBulletManager() {
	modelHandle_=ModelManager::GetInstance()->Load("Resources/Models/Bullet/bullet.gltf");
	JSON_OPEN("Resources/Data/Player/playerBullet.json");
	JSON_OBJECT("bulletProperties");
	JSON_LOAD(bulletLifeTime_);
	JSON_LOAD(bulletCoolTime_);
	JSON_LOAD(bulletSpeed_);
	JSON_LOAD(offset_);
	JSON_LOAD(reticleDistance_);
	JSON_ROOT();
	JSON_CLOSE();
	GPUParticleShaderStructs::Load("sharp", emitter_.sharp);
	GPUParticleShaderStructs::Load("crescent", emitter_.crescent);
	GPUParticleShaderStructs::Load("bullet", emitter_.bullet);
	GPUParticleShaderStructs::Load("bullet", emitter_.field);
}

void PlayerBulletManager::Initialize() {
	playerBullets_.clear();
}

void PlayerBulletManager::Update() {
	bulletTime_++;
	bulletTime_ = std::clamp(bulletTime_, 0, bulletCoolTime_);
	GPUParticleShaderStructs::BulletForGPU bullet;
	// 弾の更新と生存状態の確認
	auto iter = playerBullets_.begin();
	while (iter != playerBullets_.end()) {
		// 弾を更新する
		(*iter)->Update();

		// 弾が生存していない場合は、リストから削除する
		if (!(*iter)->GetIsAlive()) {
			
			iter = playerBullets_.erase(iter); // 削除して、次の要素を指す
		}
		else {
			bullet.collisionInfo.attribute = CollisionAttribute::PlayerBullet;
			bullet.collisionInfo.mask = CollisionAttribute::Boss;
			bullet.bullet.position = (*iter)->GetPosition();
			bullet.bullet.radius = (*iter)->GetRadius() * 5.0f;
			bullet.bullet.speed = bulletSpeed_ * 0.2f;
			bullet.emitter.particleLifeSpan.range.min = 60 * 5;
			bullet.emitter.particleLifeSpan.range.max = 60 * 5;
			++iter; // 次の弾へ移動
			gpuParticleManager_->SetBullet(bullet);
		}
	}
}

void PlayerBulletManager::DrawImGui() {
#ifdef _DEBUG
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Player")) {
		if (ImGui::TreeNode("PlayerBullet")) {
			ImGui::DragFloat3("offset", &offset_.x, 0.1f, 0.0f);
			ImGui::DragFloat("reticleDistance", &reticleDistance_, 0.1f, 0.0f);
			ImGui::DragFloat("bulletSpeed_", &bulletSpeed_, 0.1f, 0.0f);
			int time = bulletLifeTime_;
			ImGui::DragInt("bulletLifeTime_", &time, 1, 0);
			bulletLifeTime_ = time;
			time = bulletCoolTime_;
			ImGui::DragInt("bulletCoolTime_", &time, 1, 0);
			bulletCoolTime_ = time;
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/playerBullet.json");
				JSON_OBJECT("bulletProperties");
				JSON_SAVE(reticleDistance_);
				JSON_SAVE(bulletLifeTime_);
				JSON_SAVE(bulletCoolTime_);
				JSON_SAVE(bulletSpeed_);
				JSON_SAVE(offset_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	GPUParticleShaderStructs::Debug("sharp", emitter_.sharp);
	GPUParticleShaderStructs::Debug("crescent", emitter_.crescent);
	GPUParticleShaderStructs::Debug("bullet", emitter_.bullet);
	GPUParticleShaderStructs::Debug("bullet", emitter_.field);
#endif // _DEBUG
}

void PlayerBulletManager::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	// 弾
	for (auto& bullet : playerBullets_) {
		bullet->Draw(viewProjection, commandContext);
	}
}

void PlayerBulletManager::DrawDebug() {
	// 弾
	for (auto& bullet : playerBullets_) {
		bullet->DrawDebug();
	}
}

void PlayerBulletManager::Create(const WorldTransform& worldTransform) {
	if (bulletTime_ >= bulletCoolTime_) {
		bulletTime_ = 0;
		// プレイヤーの位置
		Vector3 playerPosition = MakeTranslateMatrix(worldTransform.matWorld) + RotateVector(offset_, worldTransform.rotate);
		Vector3 bulletVelocity{};
		if (Input::GetInstance()->PushKey(DIK_LSHIFT) || Input::GetInstance()->PushGamepadButton(Button::LT)) {
			// VPV合成行列
			Matrix4x4 matVPV = viewProjection_->matView_ * viewProjection_->matProjection_ * MakeViewportMatrix(0.0f, 0.0f, float(WinApp::kWindowWidth), float(WinApp::kWindowHeight), viewProjection_->nearZ_, viewProjection_->farZ_);
			// 逆行列に
			Matrix4x4 inverseVPV = Inverse(matVPV);

			// スクリーン座標
			Vector2 reticlePos = SpriteManager::GetInstance()->GetSprite(playerUI_->GetHandle(PlayerUI::kReticle))->GetPosition();
			Vector3 posNear = Vector3(reticlePos.x, reticlePos.y, 0.0f);
			Vector3 posFar = Vector3(reticlePos.x, reticlePos.y, 1.0f);

			// スクリーンからワールド座標へ
			posNear = Transform(posNear, inverseVPV);
			posFar = Transform(posFar, inverseVPV);

			// レイの方向ベクトルを計算
			Vector3 rayDirection = Normalize(posFar - posNear);

			// 3Dレティクルの位置
			Vector3 reticle3D = posNear + (rayDirection * reticleDistance_);

			// 弾の速度ベクトルを計算
			bulletVelocity = Normalize(reticle3D - playerPosition) * bulletSpeed_;
		}
		else {
			bulletVelocity = Normalize(GetZAxis(MakeRotate(worldTransform.rotate))) * bulletSpeed_;
		}
		// 弾を作成
		playerBullets_.emplace_back(std::make_unique<PlayerBullet>());
		playerBullets_.back()->SetBoss(boss_);
		playerBullets_.back()->Create(gpuParticleManager_, playerPosition, bulletVelocity, bulletLifeTime_, emitter_);
	}

}