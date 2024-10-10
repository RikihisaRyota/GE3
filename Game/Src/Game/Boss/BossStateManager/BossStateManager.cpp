#include "BossStateManager.h"

#include <numbers>

#include "../Boss.h"
#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Math/OBB.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "../Game/Src/Game/Player/Player.h"

void BossStateRoot::Initialize(CommandContext& commandContext) {
	auto boss = manager_.boss_;
	SetDesc();
	//animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("idle");
	time_ = 0.0f;
	if (manager_.GetPreState() != BossStateManager::State::kRoot) {
		data_.transformEmitter.startModelWorldMatrix = manager_.GetWorldTransform().matWorld;
		data_.transformEmitter.endModelWorldMatrix = boss->GetWorldMatrix();

		switch (manager_.GetPreState()) {
		case  BossStateManager::State::kRushAttack:
		{
			data_.transformEmitter.color.range.start = manager_.jsonData_.rushAttack.trainEmitter.color.range.end;
			data_.transformEmitter.color.range.end = boss->GetVertexEmitter().color.range.start;
		}
		break;
		default:
			break;
		}
		manager_.gpuParticleManager_->SetTransformModelEmitter(manager_.GetModelHandle(), boss->GetModelHandle(), data_.transformEmitter);
	}
}

void BossStateRoot::SetDesc() {
	data_ = manager_.jsonData_.root;
}

void BossStateRoot::Update(CommandContext& commandContext) {
	if (time_ >= 1.0f && inTransition_) {
		inTransition_ = false;
		time_ = 0.0f;
	}

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
	}
	else {
		time_ += 1.0f / data_.allFrame;
		//if (time_ >= 1.0f) {
		//	BossStateManager::State tmp = static_cast<BossStateManager::State>((rnd_.NextUIntLimit() % 2) + int(BossStateManager::State::kRushAttack));
		//	switch (tmp) {
		//	case BossStateManager::State::kRushAttack:
		//		manager_.ChangeState<BossStateRushAttack>();
		//		break;
		//	case BossStateManager::State::kSmashAttack:
		//		manager_.ChangeState<BossStateSmashAttack>();
		//		break;

		//	}
		//}
		time_ = std::fmod(time_, 1.0f);
	}

	auto boss = manager_.boss_;
	auto animation = manager_.boss_->GetAnimation();
	if (inTransition_) {

	}
	else {
		//animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
	}
}
void BossStateRoot::DebugDraw() {}

void BossStateRushAttack::Initialize(CommandContext& commandContext) {
	SetDesc();
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/train.gltf");
	railModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/rail.gltf");
	worldTransform_.Initialize();
	railWorldTransform_.Initialize();
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Boss);
	collider_->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	collider_->SetName("Boss");
	collider_->SetIsActive(true);
	time_ = 0.0f;
	SetLocation();
	auto boss = manager_.boss_;
	Vector3 worldPos = MakeTranslateMatrix(worldTransform_.matWorld);
	// VertexEmitter
	data_.trainEmitter.isAlive = false;
	data_.railEmitter.isAlive = false;
	// TransformEmitter
	data_.transformTrainEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
	data_.transformTrainEmitter.color.range.end = data_.trainEmitter.color.range.start;
	data_.transformRailEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
	// Field
	data_.headField.fieldArea.position = worldPos + RotateVector(data_.headOffset, worldTransform_.rotate);
	data_.headField.isAlive = false;
	// Smoke
	data_.smokeEmitter.isAlive = false;
	// areaTrain
	data_.areaTrainEmitter.isAlive = false;
	data_.areaTrainEmitter.emitterArea.position = worldPos;
	data_.areaTrainEmitter.modelWorldMatrix = worldTransform_.matWorld;
	data_.areaTrainEmitter.color = data_.trainEmitter.color;
	// 向きを変更
	data_.smokeEmitter.emitterArea.position = worldPos + RotateVector(data_.smokeOffset, worldTransform_.rotate);
	data_.smokeEmitter.emitterArea.aabb.area.min = RotateVector(data_.smokeEmitter.emitterArea.aabb.area.min, worldTransform_.rotate);
	data_.smokeEmitter.emitterArea.aabb.area.max = RotateVector(data_.smokeEmitter.emitterArea.aabb.area.max, worldTransform_.rotate);
	data_.smokeEmitter.velocity.range.min = RotateVector(data_.smokeEmitter.velocity.range.min, worldTransform_.rotate);
	data_.smokeEmitter.velocity.range.max = RotateVector(data_.smokeEmitter.velocity.range.max, worldTransform_.rotate);

	if (manager_.GetPreState() == BossStateManager::State::kRoot) {
		data_.transformTrainEmitter.startModelWorldMatrix = boss->GetWorldMatrix();
		data_.transformTrainEmitter.endModelWorldMatrix = worldTransform_.matWorld;
		data_.transformTrainEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
		manager_.gpuParticleManager_->SetTransformModelEmitter(boss->GetModelHandle(), modelHandle_, data_.transformTrainEmitter);
		data_.transformRailEmitter.modelWorldMatrix = railWorldTransform_.matWorld;
		GPUParticleShaderStructs::TransformAreaEmitterForCPU  emitter = data_.transformRailEmitter;
		emitter.emitterArea.position = worldTransform_.translate;
		emitter.modelWorldMatrix = railWorldTransform_.matWorld;
		manager_.gpuParticleManager_->SetTransformAreaEmitter(railModelHandle_, emitter);
	}
}

void BossStateRushAttack::SetDesc() {
	data_ = manager_.jsonData_.rushAttack;
}

void BossStateRushAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
		// 移動完了
		if (time_ >= 1.0f) {
			data_.trainEmitter.isAlive = true;
			data_.railEmitter.isAlive = true;
			data_.headField.isAlive = true;
			data_.smokeEmitter.isAlive = true;
			data_.areaTrainEmitter.isAlive = true;
			inTransition_ = false;
			time_ = 0.0f;
		}
	}

	if (!inTransition_) {
		time_ += 1.0f / data_.allFrame;
	}

	time_ = std::clamp(time_, 0.0f, 1.0f);

	if (inTransition_) {

	}
	else {
		float t = 0.0f;
		if (time_ < 0.5f) {
			t = 8.0f * time_ * time_ * time_ * time_;
		}
		else {
			t = 1.0f - std::pow(-2.0f * time_ + 2.0f, 4.0f) / 2.0f;
		}
		worldTransform_.translate.x = Lerp(data_.start.x, data_.end.x, t);
		UpdateTransform();
		data_.trainEmitter.localTransform.translate = MakeTranslateMatrix(worldTransform_.matWorld);
		data_.trainEmitter.localTransform.rotate = Inverse(MakeRotateMatrix(worldTransform_.matWorld));
		data_.railEmitter.localTransform.translate = MakeTranslateMatrix(railWorldTransform_.matWorld);
		data_.railEmitter.localTransform.rotate = Inverse(MakeRotateMatrix(railWorldTransform_.matWorld));
		data_.headField.fieldArea.position = MakeTranslateMatrix(worldTransform_.matWorld) + RotateVector(data_.headOffset, worldTransform_.rotate);;
		data_.smokeEmitter.emitterArea.position = MakeTranslateMatrix(worldTransform_.matWorld) + RotateVector(data_.smokeOffset, worldTransform_.rotate);
		data_.areaTrainEmitter.modelWorldMatrix = worldTransform_.matWorld;

	}

	if (time_ >= 1.0f && !inTransition_) {
		manager_.ChangeState<BossStateRoot>();
		data_.trainEmitter.isAlive = false;
		data_.railEmitter.isAlive = false;
		data_.headField.isAlive = false;
		data_.smokeEmitter.isAlive = false;
		data_.areaTrainEmitter.isAlive = false;
		data_.transformRailVertexEmitter.color = data_.railEmitter.color;
		data_.transformRailVertexEmitter.scale.range.start = data_.railEmitter.scale.range.start;
		data_.transformRailVertexEmitter.localTransform.translate = data_.railEmitter.localTransform.translate;
		data_.transformRailVertexEmitter.localTransform.rotate = data_.railEmitter.localTransform.rotate;

		manager_.gpuParticleManager_->SetVertexEmitter(railModelHandle_, data_.transformRailVertexEmitter);
	}
	manager_.gpuParticleManager_->SetVertexEmitter(modelHandle_, data_.trainEmitter);
	manager_.gpuParticleManager_->SetVertexEmitter(railModelHandle_, data_.railEmitter);
	//manager_.gpuParticleManager_->SetTransformAreaEmitter(modelHandle_,data_.areaTrainEmitter);
	manager_.gpuParticleManager_->SetEmitter(data_.smokeEmitter);
	manager_.gpuParticleManager_->SetField(data_.headField);
}

void BossStateRushAttack::DebugDraw() {
	collider_->DrawCollision({ 0.0f,1.0f,0.2f,1.0f });
	GPUParticleShaderStructs::DebugDraw(data_.headField);
}

void BossStateRushAttack::OnCollision(const ColliderDesc& collisionInfo) {
	collisionInfo;
}

void BossStateRushAttack::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	railWorldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
	collider_->SetSize(data_.collider.size);
	collider_->SetOrientation(worldTransform_.rotate);

}

void BossStateRushAttack::SetLocation() {

	float random = rnd_.NextFloatUnit();
	if (random >= 0.5f) {
		attackLocation_ |= AttackLocation::kRight;
	}
	else {
		attackLocation_ |= AttackLocation::kLeft;
	}
	random = rnd_.NextFloatUnit();
	if (random >= 0.5f) {
		attackLocation_ |= AttackLocation::kFront;
	}
	else {
		attackLocation_ |= AttackLocation::kBack;
	}

	Quaternion rightRotate = MakeRotateYAngleQuaternion(DegToRad(-90.0f));
	Quaternion leftRotate = MakeRotateYAngleQuaternion(DegToRad(90.0f));
	Vector3 railPos = Vector3(0.0f, data_.start.y * 0.5f, 0.0f);
	switch (attackLocation_) {
		// 右前
	case AttackLocation::kRight | AttackLocation::kFront:
	{
		data_.start.x *= -1.0f;
		data_.end.x *= -1.0f;
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		worldTransform_.rotate = rightRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		railWorldTransform_.rotate = rightRotate;
	}
	break;
	// 右後ろ
	case AttackLocation::kRight | AttackLocation::kBack:
	{
		data_.start.x *= -1.0f;
		data_.end.x *= -1.0f;
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		worldTransform_.rotate = rightRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		railWorldTransform_.rotate = rightRotate;
	}
	break;
	// 左前
	case AttackLocation::kLeft | AttackLocation::kFront:
	{
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		worldTransform_.rotate = leftRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		railWorldTransform_.rotate = leftRotate;
	}
	break;
	// 左後ろ
	case AttackLocation::kLeft | AttackLocation::kBack:
	{
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		worldTransform_.rotate = leftRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		railWorldTransform_.rotate = leftRotate;
	}
	break;
	default:
		break;
	}
	UpdateTransform();

}

void BossStateSmashAttack::Initialize(CommandContext& commandContext) {
	SetDesc();
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/hand.gltf");
	worldTransform_.Initialize();
	worldTransform_.translate.y = data_.y;
	worldTransform_.UpdateMatrix();

	SetLocation();

	time_ = 0.0f;
	auto boss = manager_.boss_;

	// VertexEmitter
	for (auto& smash : smash_) {
		smash.emitter.isAlive = false;
		smash.collider = std::make_unique<OBBCollider>();
		smash.collider->SetIsActive(false);
		smash.collider->SetName("Boss");
		smash.collider->SetCollisionAttribute(CollisionAttribute::Boss);
		smash.collider->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	}
	//　頭
	data_.smashHeadEmitter.color = data_.smashEmitter.color;

	// smashAfterimageEmitter
	data_.smashAfterimageEmitter.color = data_.smashEmitter.color;

	// TransformEmitter
	if (manager_.GetPreState() == BossStateManager::State::kRoot) {
		for (auto& smash : smash_) {
			data_.transformEmitter.startModelWorldMatrix = boss->GetWorldMatrix();
			data_.transformEmitter.endModelWorldMatrix = smash.transform.matWorld;
			data_.transformEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
			data_.transformEmitter.color.range.end = data_.smashEmitter.color.range.start;
			data_.transformEmitter.scale.range.start = boss->GetVertexEmitter().scale.range.start;
			data_.transformEmitter.scale.range.end = data_.smashEmitter.scale.range.start;

			manager_.gpuParticleManager_->SetTransformModelEmitter(boss->GetModelHandle(), modelHandle_, data_.transformEmitter);
		}
	}
}

void BossStateSmashAttack::SetDesc() {
	data_ = manager_.jsonData_.smashAttack;
}

void BossStateSmashAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
		if (time_ >= 1.0f) {
			for (auto& smash : smash_) {
				smash.emitter.isAlive = true;
				smash.collider->SetIsActive(true);
			}
			inTransition_ = false;
			time_ = 0.0f;
		}
	}

	if (!inTransition_) {
		time_ += 1.0f / data_.allFrame;
	}

	time_ = std::clamp(time_, 0.0f, 1.0f);

	if (inTransition_) {

	}
	else {
		// 最初の奴だけ別に
		for (uint32_t i = 0; i < smash_.size(); i++) {
			bool canUpdate = (i == 0) || smash_.at(i - 1).canNextMove;
			if (canUpdate) {
				smash_.at(i).Update(data_.y, data_.allFrame);

				GPUParticleShaderStructs::EmitterForCPU headEmitter{};
				GPUParticleShaderStructs::NonSharedCopy(headEmitter, data_.smashHeadEmitter);
				headEmitter.emitterArea.position = MakeTranslateMatrix(smash_.at(i).transform.matWorld) + data_.headOffset;


				GPUParticleShaderStructs::VertexEmitterForCPU smashAfterimageEmitter{};
				GPUParticleShaderStructs::NonSharedCopy(smashAfterimageEmitter, data_.smashAfterimageEmitter);
				smashAfterimageEmitter.localTransform.translate = MakeTranslateMatrix(smash_.at(i).transform.matWorld);
				smashAfterimageEmitter.localTransform.rotate = MakeRotateMatrix(smash_.at(i).transform.matWorld);

				smashAfterimageEmitter.color.range.start.max *= 0.5f;
				smashAfterimageEmitter.color.range.start.min *= 0.5f;
				//smashAfterimageEmitter.color.range.end.max *= 0.1f;
				//smashAfterimageEmitter.color.range.end.min *= 0.1f;
				manager_.gpuParticleManager_->SetVertexEmitter(modelHandle_, smashAfterimageEmitter, Mul(worldTransform_.matWorld, smash_.at(i).transform.matWorld));
			}
		}

		UpdateTransform();
	}
	// 一番最後のsmashが終わったら
	if (smash_.back().isFinish && !inTransition_) {
		for (auto& smash : smash_) {
			smash.emitter.isAlive = false;
			smash.collider->SetIsActive(false);
		}
		manager_.ChangeState<BossStateRoot>();

	}
	for (auto& smash : smash_) {
		manager_.gpuParticleManager_->SetVertexEmitter(modelHandle_, smash.emitter, smash.transform.matWorld);
	}
}

void BossStateSmashAttack::DebugDraw() {
	for (auto& smash : smash_) {
		smash.collider->DrawCollision({ 0.0f,1.0f,0.2f,1.0f });
	}
}

void BossStateSmashAttack::OnCollision(const ColliderDesc& collisionInfo) {}

void BossStateSmashAttack::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	for (auto& smash : smash_) {
		smash.transform.UpdateMatrix();
		smash.collider->SetCenter(MakeTranslateMatrix(smash.transform.matWorld));
		smash.collider->SetOrientation(smash.transform.rotate);
		smash.collider->SetSize(data_.collider.size);

	}
}

void BossStateSmashAttack::SetLocation() {
	static const uint32_t kMaxCircleNum = 3;
	static const float kCircleRadius = 5.0f;
	static const float kObjectRadius = 6.0f;
	// 円の数
	for (uint32_t circleNum = 1; circleNum <= kMaxCircleNum; ++circleNum) {
		// 現在の円の半径
		float currentRadius = kCircleRadius * circleNum;

		// 円周の長さを計算
		float circumference = std::numbers::pi_v<float> *2.0f * currentRadius;

		// オブジェクトの数を計算
		uint32_t divisionNum = static_cast<uint32_t>(circumference / kObjectRadius);

		// 円周上に等間隔にポジションをセット
		for (uint32_t i = 0; i < divisionNum; ++i) {
			CreateSmash();
			auto& smash = smash_.back();
			smash.transform.translate.x = std::cosf(DegToRad(360.0f * i / divisionNum)) * currentRadius;
			smash.transform.translate.y = 0.0f;//data_.y;
			smash.transform.translate.z = std::sinf(DegToRad(360.0f * i / divisionNum)) * currentRadius;
			// 円の中心に(Quaternion)
			Vector3 direction = Vector3(smash.transform.translate.x, 0.0f, smash.transform.translate.z).Normalize();
			smash.transform.rotate = MakeLookRotation(direction);
			smash.transform.UpdateMatrix();
			smash.start = smash.transform.translate;
			smash.end = { smash.start.x, 0.0f, smash.start.z };
		}
	}

}

void BossStateSmashAttack::CreateSmash() {
	SmashDesc desc{};
	desc.collider = std::make_unique<OBBCollider>();
	desc.collider->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	desc.collider->SetCollisionAttribute(CollisionAttribute::Boss);
	desc.collider->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	desc.collider->SetName("Boss");
	desc.collider->SetIsActive(false);
	desc.transform.Initialize();
	desc.transform.parent_ = &worldTransform_;
	desc.transform.UpdateMatrix();
	// カウント
	uint32_t emitterCount = desc.emitter.emitterCount;
	desc.emitter = data_.smashEmitter;
	desc.emitter.emitterCount = emitterCount;
	desc.start = Vector3(0.0f, data_.y, 0.0f);
	desc.end = Vector3(0.0f, 0.0f, 0.0f);

	smash_.emplace_back(std::move(desc));
}

void BossStateHomingAttack::Initialize(CommandContext& commandContext) {
	auto boss = manager_.boss_;
	SetDesc();
	inTransition_ = true;
	time_ = 0.0f;

	// 弾初期化
	InitializeBullet();

	worldTransform_.Initialize();
	worldTransform_.translate = boss->GetWorldTranslate();
	worldTransform_.translate += data_.start;
	worldTransform_.UpdateMatrix();
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/cannon.gltf");

	// Vertex
	data_.homingEmitter.isAlive = false;

	// Transform
	data_.transformEmitter.startModelWorldMatrix = boss->GetWorldMatrix();
	data_.transformEmitter.endModelWorldMatrix = worldTransform_.matWorld;
	data_.transformEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
	data_.transformEmitter.color.range.end = data_.homingEmitter.color.range.start;
	data_.transformEmitter.scale.range.start = boss->GetVertexEmitter().scale.range.start;
	data_.transformEmitter.scale.range.end = data_.homingEmitter.scale.range.start;
	manager_.gpuParticleManager_->SetTransformModelEmitter(boss->GetModelHandle(), modelHandle_, data_.transformEmitter);
}

void BossStateHomingAttack::SetDesc() {
	data_ = manager_.jsonData_.homingAttack;
}

void BossStateHomingAttack::InitializeBullet() {
	// BulletEmitter
	data_.bulletEmitter.isAlive = false;
	createBulletTime_ = 0;
	for (auto& [fired, bullet] : bullets_) {
		bullet = std::make_unique<Bullet>();
		bullet->Initialize(data_.bulletRadius);
		bullet->SetEmitter(data_.bulletEmitter,data_.homingExplosionEmitter);
		bullet->SetField(data_.bulletField, data_.homingExplosionField);
		fired = false;
	}
}

void BossStateHomingAttack::Update(CommandContext& commandContext) {
	if (time_ >= 1.0f && inTransition_) {
		data_.homingEmitter.isAlive = true;
		inTransition_ = false;
		time_ = 0.0f;
	}

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
	}
	else {
		time_ += 1.0f / data_.allFrame;

		time_ = std::clamp(time_, 0.0f, 1.0f);
	}

	if (inTransition_) {

	}
	else {
		worldTransform_.rotate = MakeLookRotation((MakeTranslateMatrix(worldTransform_.matWorld) - manager_.player_->GetWorldTranslate()).Normalize());
		worldTransform_.UpdateMatrix();


		// 弾の更新
		UpdateBullet();

		// 全ての弾が発射されていて、かつ死んでいるかを確認
		bool isBulletAlive = std::all_of(bullets_.begin(), bullets_.end(), [](const auto& bulletPair) {
			const auto& [fired, bullet] = bulletPair;
			return fired && !bullet->GetIsAlive();  // 発射済みかつ死んでいるかを確認
			});

		if (isBulletAlive) {
			manager_.ChangeState<BossStateRoot>();
			data_.transformEmitter.isAlive = false;
			data_.homingEmitter.isAlive = false;
		}

	}
	manager_.gpuParticleManager_->SetVertexEmitter(modelHandle_, data_.homingEmitter, worldTransform_.matWorld);
}

void BossStateHomingAttack::UpdateBullet() {

	for (auto& bullet : bullets_) {
		if (bullet.second->GetIsAlive()) {
			bullet.second->Update();
			manager_.gpuParticleManager_->SetEmitter(bullet.second->GetBulletEmitter());
			manager_.gpuParticleManager_->SetEmitter(bullet.second->GetExplosionEmitter());
			manager_.gpuParticleManager_->SetField(bullet.second->GetBulletField());
			manager_.gpuParticleManager_->SetField(bullet.second->GetExplosionFieldField());
		}
	}

	// 生成
	if (createBulletTime_ <= 0) {
		for (auto& [fired, bullet] : bullets_) {
			if (!fired && !bullet->GetIsAlive()) {
				auto boss = manager_.boss_;
				auto player = manager_.player_;
				Vector3 position = MakeTranslateMatrix(worldTransform_.matWorld) + data_.bulletOffset;
				bullet->Create(position, player->GetWorldTranslate(), data_.bulletHeightOffset, data_.bulletSpeed);
				// 発射済みにする
				fired = true;
				// Particleセット
				data_.fireEmitter.emitterArea.position = position;
				manager_.gpuParticleManager_->SetEmitter(data_.fireEmitter);
				break;
			}
		}
		createBulletTime_ = data_.createBulletInterval;
	}
	createBulletTime_--;
}


void BossStateHomingAttack::DebugDraw() {
	for (auto& bullet : bullets_) {
		if (bullet.second->GetIsAlive()) {
			bullet.second->DebugDraw();
		}
	}

}


void BossStateManager::Initialize() {
	JSON_OPEN("Resources/Data/Boss/bossState.json");
	JSON_OBJECT("Root");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.root.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.root.transitionFrame);
	JSON_ROOT();

	JSON_OBJECT("RushAttack");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.rushAttack.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.rushAttack.transitionFrame);
	JSON_PARENT();
	JSON_OBJECT("Collider");
	JSON_LOAD_BY_NAME("size", jsonData_.rushAttack.collider.size);
	JSON_PARENT();
	JSON_OBJECT("Properties");
	JSON_LOAD_BY_NAME("frontAndBackOffset", jsonData_.rushAttack.frontAndBackOffset);
	JSON_LOAD_BY_NAME("start", jsonData_.rushAttack.start);
	JSON_LOAD_BY_NAME("end", jsonData_.rushAttack.end);
	JSON_LOAD_BY_NAME("headOffset", jsonData_.rushAttack.headOffset);
	JSON_LOAD_BY_NAME("smokeOffset", jsonData_.rushAttack.smokeOffset);
	JSON_PARENT();
	JSON_ROOT();

	JSON_OBJECT("SmashAttack");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.smashAttack.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.smashAttack.transitionFrame);
	JSON_PARENT();
	JSON_OBJECT("Collider");
	JSON_LOAD_BY_NAME("size", jsonData_.smashAttack.collider.size);
	JSON_PARENT();
	JSON_OBJECT("Properties");
	JSON_LOAD_BY_NAME("height", jsonData_.smashAttack.y);
	JSON_LOAD_BY_NAME("count", jsonData_.smashAttack.smashCount);
	JSON_LOAD_BY_NAME("headOffset", jsonData_.smashAttack.headOffset);
	JSON_PARENT();
	JSON_ROOT();

	JSON_OBJECT("HomingAttack");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.homingAttack.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.homingAttack.transitionFrame);
	JSON_PARENT();
	JSON_OBJECT("Properties");
	JSON_LOAD_BY_NAME("start", jsonData_.homingAttack.start);
	JSON_LOAD_BY_NAME("bulletOffset", jsonData_.homingAttack.bulletOffset);
	JSON_LOAD_BY_NAME("bulletHeightOffset", jsonData_.homingAttack.bulletHeightOffset);
	JSON_LOAD_BY_NAME("createBulletInterval", jsonData_.homingAttack.createBulletInterval);
	JSON_LOAD_BY_NAME("bulletSpeed", jsonData_.homingAttack.bulletSpeed);
	JSON_LOAD_BY_NAME("bulletRadius", jsonData_.homingAttack.bulletRadius);
	JSON_PARENT();
	JSON_ROOT();

	JSON_CLOSE();
	GPUParticleShaderStructs::Load("root", jsonData_.root.transformEmitter);

	GPUParticleShaderStructs::Load("train", jsonData_.rushAttack.trainEmitter);
	GPUParticleShaderStructs::Load("train", jsonData_.rushAttack.transformTrainEmitter);
	GPUParticleShaderStructs::Load("areaTrainEmitter", jsonData_.rushAttack.areaTrainEmitter);
	GPUParticleShaderStructs::Load("rail", jsonData_.rushAttack.railEmitter);
	GPUParticleShaderStructs::Load("transformRailVertexEmitter", jsonData_.rushAttack.transformRailVertexEmitter);
	GPUParticleShaderStructs::Load("rail", jsonData_.rushAttack.transformRailEmitter);
	GPUParticleShaderStructs::Load("headField", jsonData_.rushAttack.headField);
	GPUParticleShaderStructs::Load("smokeEmitter", jsonData_.rushAttack.smokeEmitter);
	GPUParticleShaderStructs::Load("headEmitter", jsonData_.rushAttack.headEmitter);


	GPUParticleShaderStructs::Load("smash", jsonData_.smashAttack.smashEmitter);
	GPUParticleShaderStructs::Load("smashAfterimageEmitter", jsonData_.smashAttack.smashAfterimageEmitter);
	GPUParticleShaderStructs::Load("smash", jsonData_.smashAttack.transformEmitter);
	GPUParticleShaderStructs::Load("smashHeadEmitter", jsonData_.smashAttack.smashHeadEmitter);

	GPUParticleShaderStructs::Load("homing", jsonData_.homingAttack.transformEmitter);
	GPUParticleShaderStructs::Load("homing", jsonData_.homingAttack.homingEmitter);
	GPUParticleShaderStructs::Load("bulletEmitter", jsonData_.homingAttack.bulletEmitter);
	GPUParticleShaderStructs::Load("fireEmitter", jsonData_.homingAttack.fireEmitter);
	GPUParticleShaderStructs::Load("bulletField", jsonData_.homingAttack.bulletField);
	GPUParticleShaderStructs::Load("homingExplosionEmitter", jsonData_.homingAttack.homingExplosionEmitter);
	GPUParticleShaderStructs::Load("homingExplosionField", jsonData_.homingAttack.homingExplosionField);

	activeStateEnum_ = kRoot;
	standbyStateEnum_ = kRoot;
	ChangeState<BossStateRoot>();
}

void BossStateManager::Update(CommandContext& commandContext) {
	if (standbyState_) {
		preStateEnum_ = activeStateEnum_;
		activeStateEnum_ = standbyStateEnum_;
		standbyStateEnum_ = kNone;
		activeState_ = std::move(standbyState_);
		activeState_->Initialize(commandContext);
	}

	if (activeState_) {
		activeState_->Update(commandContext);
#ifdef _DEBUG
		activeState_->DebugDraw();
#endif // _DEBUG

	}
}

void BossStateManager::DrawImGui() {
#ifdef _DEBUG


	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		if (ImGui::TreeNode("BossStateManager")) {
			// ステートを変更するImGui::Comboの作成
			// コンボボックスに渡すための const char* 配列を生成
			std::vector<const char*> stateNamesCStr;
			for (const auto& state : stateNames_) {
				stateNamesCStr.push_back(state.c_str());
			}
			int currentState = static_cast<int>(activeStateEnum_);

			// ステートを変更するImGui::Comboの作成
			if (ImGui::Combo("Change State", &currentState, stateNamesCStr.data(), int(stateNamesCStr.size()))) {
				switch (currentState) {
				case kRoot:
					ChangeState<BossStateRoot>();
					break;
				case kRushAttack:
					ChangeState<BossStateRushAttack>();
					break;
				case kSmashAttack:
					ChangeState<BossStateSmashAttack>();
					break;
				case kHomingAttack:
					ChangeState<BossStateHomingAttack>();
					break;
				default:
					break;
				}
			}
			if (ImGui::TreeNode("Root")) {
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.root.allFrame, 0.1f);
					ImGui::DragFloat("transitionFrame", &jsonData_.root.transitionFrame, 0.1f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("RushAttack")) {
				if (ImGui::TreeNode("Properties")) {
					ImGui::DragFloat("frontAndBackOffset", &jsonData_.rushAttack.frontAndBackOffset, 0.1f, 0.0f);
					ImGui::DragFloat3("start", &jsonData_.rushAttack.end.x, 0.1f, 0.0f);
					ImGui::DragFloat3("end", &jsonData_.rushAttack.start.x, 0.1f, 0.0f);
					ImGui::DragFloat3("headOffset", &jsonData_.rushAttack.headOffset.x, 0.1f, 0.0f);
					ImGui::DragFloat3("smokeOffset", &jsonData_.rushAttack.smokeOffset.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Collider")) {
					ImGui::DragFloat3("size", &jsonData_.rushAttack.collider.size.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.rushAttack.allFrame, 0.1f, 0.0f);
					ImGui::DragFloat("transitionFrame", &jsonData_.rushAttack.transitionFrame, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("SmashAttack")) {
				if (ImGui::TreeNode("Properties")) {
					ImGui::DragFloat("height", &jsonData_.smashAttack.y, 0.1f, 0.0f);
					ImGui::DragFloat3("headOffset", &jsonData_.smashAttack.headOffset.x, 0.1f, 0.0f);
					int i = jsonData_.smashAttack.smashCount;
					ImGui::DragInt("count", &i, 1, 0, 10);
					jsonData_.smashAttack.smashCount = i;
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Collider")) {
					ImGui::DragFloat3("size", &jsonData_.smashAttack.collider.size.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.smashAttack.allFrame, 0.1f, 0.0f);
					ImGui::DragFloat("transitionFrame", &jsonData_.smashAttack.transitionFrame, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("HomingAttack")) {
				if (ImGui::TreeNode("Properties")) {
					ImGui::DragFloat3("start", &jsonData_.homingAttack.start.x, 0.1f, 0.0f);
					ImGui::DragFloat3("bulletOffset", &jsonData_.homingAttack.bulletOffset.x, 0.1f, 0.0f);
					ImGui::DragFloat3("bulletHeightOffset", &jsonData_.homingAttack.bulletHeightOffset.x, 0.1f, 0.0f);
					ImGui::DragInt("createBulletInterval", &jsonData_.homingAttack.createBulletInterval, 1, 0);
					ImGui::DragFloat("bulletSpeed", &jsonData_.homingAttack.bulletSpeed, 0.1f, 0.0f);
					ImGui::DragFloat("bulletRadius", &jsonData_.homingAttack.bulletRadius, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.homingAttack.allFrame, 0.1f, 0.0f);
					ImGui::DragFloat("transitionFrame", &jsonData_.homingAttack.transitionFrame, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}

			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/bossState.json");
				JSON_OBJECT("Root");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.root.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.root.transitionFrame);
				JSON_ROOT();

				JSON_OBJECT("RushAttack");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.rushAttack.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.rushAttack.transitionFrame);
				JSON_PARENT();
				JSON_OBJECT("Collider");
				JSON_SAVE_BY_NAME("size", jsonData_.rushAttack.collider.size);
				JSON_PARENT();
				JSON_OBJECT("Properties");
				JSON_SAVE_BY_NAME("frontAndBackOffset", jsonData_.rushAttack.frontAndBackOffset);
				JSON_SAVE_BY_NAME("start", jsonData_.rushAttack.start);
				JSON_SAVE_BY_NAME("end", jsonData_.rushAttack.end);
				JSON_SAVE_BY_NAME("headOffset", jsonData_.rushAttack.headOffset);
				JSON_SAVE_BY_NAME("smokeOffset", jsonData_.rushAttack.smokeOffset);
				JSON_PARENT();
				JSON_ROOT();

				JSON_OBJECT("SmashAttack");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.smashAttack.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.smashAttack.transitionFrame);
				JSON_PARENT();
				JSON_OBJECT("Collider");
				JSON_SAVE_BY_NAME("size", jsonData_.smashAttack.collider.size);
				JSON_PARENT();
				JSON_OBJECT("Properties");
				JSON_SAVE_BY_NAME("height", jsonData_.smashAttack.y);
				JSON_SAVE_BY_NAME("count", jsonData_.smashAttack.smashCount);
				JSON_SAVE_BY_NAME("headOffset", jsonData_.smashAttack.headOffset);
				JSON_PARENT();
				JSON_ROOT();

				JSON_OBJECT("HomingAttack");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.homingAttack.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.homingAttack.transitionFrame);
				JSON_PARENT();
				JSON_OBJECT("Properties");
				JSON_SAVE_BY_NAME("start", jsonData_.homingAttack.start);
				JSON_SAVE_BY_NAME("bulletOffset", jsonData_.homingAttack.bulletOffset);
				JSON_SAVE_BY_NAME("bulletHeightOffset", jsonData_.homingAttack.bulletHeightOffset);
				JSON_SAVE_BY_NAME("createBulletInterval", jsonData_.homingAttack.createBulletInterval);
				JSON_SAVE_BY_NAME("bulletSpeed", jsonData_.homingAttack.bulletSpeed);
				JSON_SAVE_BY_NAME("bulletRadius", jsonData_.homingAttack.bulletRadius);
				JSON_PARENT();
				JSON_ROOT();

				JSON_CLOSE();

				Initialize();

				//GPUParticleShaderStructs::Save("root", jsonData_.root.transformEmitter);
				//GPUParticleShaderStructs::Save("rushAttack", jsonData_.rushAttack.trainEmitter);
				//GPUParticleShaderStructs::Save("rushAttack", jsonData_.rushAttack.transformTrainEmitter);
				//GPUParticleShaderStructs::Save("rail", jsonData_.rushAttack.railEmitter);
				//GPUParticleShaderStructs::Save("rail", jsonData_.rushAttack.transformRailEmitter);
				//GPUParticleShaderStructs::Save("smash", jsonData_.smashAttack.smashEmitter);
				//GPUParticleShaderStructs::Save("smash", jsonData_.smashAttack.transformEmitter);
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	GPUParticleShaderStructs::Debug("root", jsonData_.root.transformEmitter);
	GPUParticleShaderStructs::Debug("train", jsonData_.rushAttack.trainEmitter);
	GPUParticleShaderStructs::Debug("train", jsonData_.rushAttack.transformTrainEmitter);
	GPUParticleShaderStructs::Debug("areaTrainEmitter", jsonData_.rushAttack.areaTrainEmitter);
	GPUParticleShaderStructs::Debug("rail", jsonData_.rushAttack.railEmitter);
	GPUParticleShaderStructs::Debug("rail", jsonData_.rushAttack.transformRailEmitter);
	GPUParticleShaderStructs::Debug("transformRailVertexEmitter", jsonData_.rushAttack.transformRailVertexEmitter);
	GPUParticleShaderStructs::Debug("headField", jsonData_.rushAttack.headField);
	GPUParticleShaderStructs::Debug("smokeEmitter", jsonData_.rushAttack.smokeEmitter);
	GPUParticleShaderStructs::Debug("headEmitter", jsonData_.rushAttack.headEmitter);

	GPUParticleShaderStructs::Debug("smash", jsonData_.smashAttack.smashEmitter);
	GPUParticleShaderStructs::Debug("smashAfterimageEmitter", jsonData_.smashAttack.smashAfterimageEmitter);
	GPUParticleShaderStructs::Debug("smash", jsonData_.smashAttack.transformEmitter);
	GPUParticleShaderStructs::Debug("smashHeadEmitter", jsonData_.smashAttack.smashHeadEmitter);

	GPUParticleShaderStructs::Debug("homing", jsonData_.homingAttack.transformEmitter);
	GPUParticleShaderStructs::Debug("homing", jsonData_.homingAttack.homingEmitter);
	GPUParticleShaderStructs::Debug("bulletEmitter", jsonData_.homingAttack.bulletEmitter);
	GPUParticleShaderStructs::Debug("bulletField", jsonData_.homingAttack.bulletField);
	GPUParticleShaderStructs::Debug("fireEmitter", jsonData_.homingAttack.fireEmitter);
	GPUParticleShaderStructs::Debug("homingExplosionEmitter", jsonData_.homingAttack.homingExplosionEmitter);
	GPUParticleShaderStructs::Debug("homingExplosionField", jsonData_.homingAttack.homingExplosionField);

#endif // _DEBUG
}
// 特定の型に対する GetStateEnum の特殊化
template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateRoot>() {
	return kRoot;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateRushAttack>() {
	return kRushAttack;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateSmashAttack>() {
	return kSmashAttack;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateHomingAttack>() {
	return kHomingAttack;
}

void BossStateSmashAttack::SmashDesc::Update(float startPosY, float allFrame) {
	time += 1.0f / allFrame;

	float t = time * time * time * time;

	transform.translate.y = std::lerp(0.0f, -startPosY, t);

	time = std::clamp(time, 0.0f, 1.0f);

	if (time >= 0.2f) {
		canNextMove = true;
	}
	if (time >= 1.0f) {
		isFinish = true;
	}
}

void BossStateHomingAttack::Bullet::Initialize(float radius) {
	worldTransform_.Initialize();
	isAlive_ = false;

	collider = std::make_unique<SphereCollider>();

	collider->SetRadius(radius);
	collider->SetName("Boss");
	collider->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider->SetCollisionAttribute(CollisionAttribute::Boss);
	collider->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	collider->SetIsActive(isAlive_);
}

void BossStateHomingAttack::Bullet::Create(const Vector3& start, const Vector3& end, const Vector3& offset, float allFrame) {
	worldTransform_.Initialize();
	start_ = start;
	end_ = end;
	worldTransform_.translate = start_;
	time_ = 0.0f;
	allFrame_ = allFrame;
	bulletHeightOffset_ = offset;
	isAlive_ = true;
	bulletEmitter_.isAlive = isAlive_;
	bulletField_.isAlive = isAlive_;
	collider->SetIsActive(isAlive_);
}

void BossStateHomingAttack::Bullet::Update() {
	time_ += 1.0f / allFrame_;
	worldTransform_.translate = CubicBezier(start_, Lerp(start_, end_, 0.5f) + bulletHeightOffset_, end_, time_);
	worldTransform_.UpdateMatrix();
	Vector3 worldPos = MakeTranslateMatrix(worldTransform_.matWorld);
	collider->SetCenter(worldPos);
	// Particleセット
	bulletEmitter_.emitterArea.position = worldPos;
	bulletField_.fieldArea.position = worldPos;
	explosionEmitter_.emitterArea.position = worldPos;
	explosionField_.fieldArea.position = worldPos;
	// 死ぬ
	if (time_ >= 1.0f) {
		isAlive_ = false;
		bulletEmitter_.isAlive = isAlive_;
		bulletField_.isAlive = isAlive_;
		explosionEmitter_.isAlive = !isAlive_;
		explosionField_.isAlive = !isAlive_;
		collider->SetIsActive(isAlive_);
	}
}

void BossStateHomingAttack::Bullet::DebugDraw() {
	collider->DrawCollision({ 0.0f,1.0f,0.2f,1.0f });
}


void BossStateHomingAttack::Bullet::OnCollision(const ColliderDesc& desc) {
	desc;
}
