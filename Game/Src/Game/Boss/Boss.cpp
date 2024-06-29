#include "Boss.h"

#include <numbers>

#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"

#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Texture/TextureManager.h"

#include "Engine/Json/JsonUtils.h"

Boss::Boss() {
	bossModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/boss.gltf");
	animation_.Initialize("Resources/Animation/Boss/animation.gltf", bossModelHandle_);
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	bossStateManager_ = std::make_unique<BossStateManager>();
	bossStateManager_->SetBoss(this);
	worldTransform_.Initialize();
	animationTransform_.Initialize();

	bossHP_ = std::make_unique<BossHP>();


	JSON_OPEN("Resources/Data/Boss/bossCollision.json");
#pragma region コライダー
	for (auto& joint : animation_.skeleton.joints) {
		if (!joint.parent) {
			continue;
		}
		std::string jointName = EraseName(joint.name, "mixamorig:");

		JSON_OBJECT("bossCollider");
		JSON_LOAD_BY_NAME(jointName, colliderSize_[jointName]);
		JSON_ROOT();
		bossCollider_[jointName] = std::make_unique<BossCollider>();
		bossCollider_[jointName]->body = std::make_unique<CapsuleCollider>();
		bossCollider_[jointName]->attack = std::make_unique<CapsuleCollider>();
		bossCollider_[jointName]->body->SetName("Boss");
		bossCollider_[jointName]->attack->SetName("BossAttack");
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * animationTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * animationTransform_.matWorld;

		Vector3 pos = MakeTranslateMatrix(worldMatrix);
		Vector3 parentPos = MakeTranslateMatrix(parentMatrix);
		//Vector3 born = (MakeTranslateMatrix(worldMatrix) - MakeTranslateMatrix(parentMatrix));
		//Vector3 center = MakeTranslateMatrix(parentMatrix) + born * 0.5f;
		//Quaternion orientation = MakeLookRotation(born.Normalized());
		//Vector3 size = { born.Length() * colliderSize_[joint.name], born.Length() * colliderSize_[joint.name],born.Length() };

		bossCollider_[jointName]->body->SetSegment(Segment(pos, parentPos));
		bossCollider_[jointName]->attack->SetSegment(Segment(pos, parentPos));
		bossCollider_[jointName]->body->SetRadius(colliderSize_[jointName]);
		bossCollider_[jointName]->attack->SetRadius(colliderSize_[jointName]);
		bossCollider_[jointName]->body->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollisionBody(collisionInfo); });
		bossCollider_[jointName]->attack->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollisionAttack(collisionInfo); });
		bossCollider_[jointName]->body->SetCollisionAttribute(CollisionAttribute::BossBody);
		bossCollider_[jointName]->attack->SetCollisionAttribute(CollisionAttribute::BossAttack);
		bossCollider_[jointName]->body->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
		bossCollider_[jointName]->body->SetCollisionMask(CollisionAttribute::Player);
		bossCollider_[jointName]->color = { 0.0f,1.0f,0.0f,1.0f };
		if (colliderSize_[jointName] != 0.0f) {
			bossCollider_[jointName]->body->SetIsActive(true);
			bossCollider_[jointName]->attack->SetIsActive(false);
		}
		else {
			bossCollider_[jointName]->body->SetIsActive(false);
			bossCollider_[jointName]->attack->SetIsActive(false);
		}
	}
#pragma endregion
	JSON_ROOT();
	// コライダータイプの初期化
	for (auto& stateName : bossStateManager_->stateNames_) {
		colliderType_[stateName];
		selectedNodeNameIndices_[stateName] = -1;
		selectedEntryNodeNameIndices_[stateName] = -1;
	}
	std::vector<std::string> colliderNames{};
	for (auto& type : colliderType_) {
		JSON_OBJECT("colliderTypes");
		JSON_OBJECT(type.first);
		JSON_OBJECT("colliderNames");
		for (auto& name : bossCollider_) {
			std::string nodeName = name.first;
			if (JSON_LOAD_BY_NAME(nodeName, nodeName)) {
				colliderNames.emplace_back(nodeName);
			}
		}
		type.second = colliderNames;
		colliderNames.clear();
		JSON_ROOT();
	}
	JSON_LOAD(attackColor_.range.start.min);
	JSON_LOAD(attackColor_.range.start.max);
	JSON_LOAD(attackColor_.range.end.min);
	JSON_LOAD(attackColor_.range.end.max);
	JSON_LOAD(defaultColor_.range.start.min);
	JSON_LOAD(defaultColor_.range.start.max);
	JSON_LOAD(defaultColor_.range.end.min);
	JSON_LOAD(defaultColor_.range.end.max);

	JSON_ROOT();

	JSON_CLOSE();
	JSON_OPEN("Resources/Data/Boss/boss.json");
	JSON_OBJECT("bossProperties");
	JSON_LOAD(offset_);
	JSON_LOAD(animationWorldTransformOffset_);
	JSON_ROOT();
	JSON_CLOSE();
	for (auto& colliderName : colliderSize_) {
		GPUParticleShaderStructs::Load(colliderName.first, emitters_[colliderName.first]);
	}
	GPUParticleShaderStructs::Load("boss", meshEmitterDesc_);
	GPUParticleShaderStructs::Load("boss", vertexEmitterDesc_);
}

void Boss::Initialize() {
	bossStateManager_->Initialize();
	bossHP_->Initialize();
	worldTransform_.Reset();
	worldTransform_.translate = offset_;
	worldTransform_.rotate = MakeRotateYAngleQuaternion(DegToRad(180.0f));
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	animationTransform_.translate = animationWorldTransformOffset_;
	UpdateTransform();
	UpdateCollider();
}

void Boss::Update(CommandContext& commandContext) {
	UpdateGPUParticle();
	bossStateManager_->Update(commandContext);
	UpdateTransform();
	UpdateCollider();
}

void Boss::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//ModelManager::GetInstance()->Draw(animationTransform_, animation_, viewProjection, bossModelHandle_, commandContext);

	//gpuParticleManager_->CreateMeshParticle(bossModelHandle_, animation_, worldTransform_, meshEmitterDesc_, commandContext);
	//gpuParticleManager_->CreateVertexParticle(bossModelHandle_, animation_, worldTransform_, vertexEmitterDesc_, commandContext);

}

void Boss::DrawImGui() {
#ifdef _DEBUG
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		auto& color = ModelManager::GetInstance()->GetModel(bossModelHandle_).GetMaterialColor();
		ImGui::DragFloat3("color", &color.x, 0.1f, 0.0f, 1.0f);
		ModelManager::GetInstance()->GetModel(bossModelHandle_).SetMaterialColor(color);
		ImGui::DragFloat3("translate", &worldTransform_.translate.x, 0.1f);
		ImGui::DragFloat3("animationTranslate", &animationTransform_.translate.x, 0.1f);
		ImGui::Text("matWorld: x:%f,y:%f,z:%f", MakeTranslateMatrix(worldTransform_.matWorld).x, MakeTranslateMatrix(worldTransform_.matWorld).y, MakeTranslateMatrix(worldTransform_.matWorld).z, 0.1f);
		if (ImGui::TreeNode("Properties")) {
			ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
			ImGui::DragFloat3("AnimationWorldTransformOffset_", &animationWorldTransformOffset_.x, 0.1f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/boss.json");
				JSON_OBJECT("bossProperties");
				JSON_SAVE(offset_);
				JSON_SAVE(animationWorldTransformOffset_);
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collision")) {
			if (ImGui::TreeNode("Color")) {

				if (ImGui::TreeNode("AttackColor")) {
					if (ImGui::TreeNode("Start")) {
						ImGui::DragFloat4("Min", &attackColor_.range.start.min.x, 0.01f, 0.0f, 1.0f);
						ImGui::DragFloat4("Max", &attackColor_.range.start.max.x, 0.01f, 0.0f, 1.0f);
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("End")) {
						ImGui::DragFloat4("Min", &attackColor_.range.end.min.x, 0.01f, 0.0f, 1.0f);
						ImGui::DragFloat4("Max", &attackColor_.range.end.max.x, 0.01f, 0.0f, 1.0f);
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
				if (ImGui::TreeNode("DefaultColor")) {
					if (ImGui::TreeNode("Start")) {
						ImGui::DragFloat4("Min", &defaultColor_.range.start.min.x, 0.01f, 0.0f, 1.0f);
						ImGui::DragFloat4("Max", &defaultColor_.range.start.max.x, 0.01f, 0.0f, 1.0f);
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("End")) {
						ImGui::DragFloat4("Min", &defaultColor_.range.end.min.x, 0.01f, 0.0f, 1.0f);
						ImGui::DragFloat4("Max", &defaultColor_.range.end.max.x, 0.01f, 0.0f, 1.0f);
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
				if (ImGui::Button("Save")) {
					JSON_OPEN("Resources/Data/Boss/bossCollision.json");
					JSON_SAVE(attackColor_.range.start.min);
					JSON_SAVE(attackColor_.range.start.max);
					JSON_SAVE(attackColor_.range.end.min);
					JSON_SAVE(attackColor_.range.end.max);
					JSON_SAVE(defaultColor_.range.start.min);
					JSON_SAVE(defaultColor_.range.start.max);
					JSON_SAVE(defaultColor_.range.end.min);
					JSON_SAVE(defaultColor_.range.end.max);
					JSON_CLOSE();

				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Size")) {
				for (auto& size : colliderSize_) {
					ImGui::DragFloat(size.first.c_str(), &size.second, 0.1f, 0.0f);
				}
				if (ImGui::Button("Save")) {
					JSON_OPEN("Resources/Data/Boss/bossCollision.json");
					JSON_OBJECT("bossCollider");
					for (auto& size : colliderSize_) {
						JSON_SAVE_BY_NAME(size.first, size.second);
					}
					JSON_ROOT();
					JSON_CLOSE();
				}
				ImGui::TreePop();
			}


			if (ImGui::TreeNode("ColliderType")) {
				if (ImGui::TreeNode("NodeName")) {
					// bossCollider_ のキーからノード名を取得してリストボックスに表示
					std::vector<const char*> colliderNodeNames;
					for (const auto& colliderNode : bossCollider_) {
						colliderNodeNames.push_back(colliderNode.first.c_str());
					}
					// 各タイプについてループ
					for (auto& type : colliderType_) {
						if (ImGui::TreeNode(type.first.c_str())) {
							// 現在登録してあるノード名を表示するリストボックス
							std::vector<const char*> nodeNames;
							for (const auto& nodeName : type.second) {
								bossCollider_[nodeName]->color = { 1.0f,1.0f,0.0f,1.0f };
								nodeNames.push_back(nodeName.c_str());
							}

							int& selectedEntryNodeNameIndex = selectedEntryNodeNameIndices_[type.first];

							ImGui::Combo("Entry Node Names", &selectedEntryNodeNameIndex, nodeNames.data(), int(nodeNames.size()));

							// 選択されたノード名を削除するボタン
							if (selectedEntryNodeNameIndex >= 0 && selectedEntryNodeNameIndex < nodeNames.size()) {
								bossCollider_[colliderNodeNames[selectedEntryNodeNameIndex]]->color = { 0.0f,0.0f,1.0f,1.0f };
								if (ImGui::Button("Remove Selected NodeName")) {
									type.second.erase(type.second.begin() + selectedEntryNodeNameIndex);
									//selectedEntryNodeNameIndex = -1;  // 削除後、選択状態をクリア
								}
							}

							// ノード名を追加
							if (ImGui::TreeNode("Add NodeName")) {


								int& selectedNodeNameIndex = selectedNodeNameIndices_[type.first];

								// ノード表示
								ImGui::Combo("Node Names", &selectedNodeNameIndex, colliderNodeNames.data(), int(colliderNodeNames.size()));
								// 選択されたノード名を追加するボタン
								if (selectedNodeNameIndex >= 0 && selectedNodeNameIndex < colliderNodeNames.size()) {
									bossCollider_[colliderNodeNames[selectedNodeNameIndex]]->color = { 0.0f,0.0f,1.0f,1.0f };
									if (ImGui::Button("Add")) {
										std::string newName(colliderNodeNames[selectedNodeNameIndex]);
										if (!newName.empty()) {
											type.second.emplace_back(newName);
											//selectedNodeNameIndex = -1;
										}
									}
								}
								ImGui::TreePop();
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}

				if (ImGui::Button("Save")) {
					JSON_OPEN("Resources/Data/Boss/bossCollision.json");
					for (auto& type : colliderType_) {
						JSON_OBJECT("colliderTypes");
						JSON_OBJECT(type.first);
						JSON_OBJECT("colliderNames");
						for (auto& name : type.second) {
							JSON_SAVE_BY_NAME(name, name);
						}
						JSON_ROOT();
					}
					JSON_ROOT();
					JSON_LOAD(attackColor_.range.start.min);
					JSON_LOAD(attackColor_.range.start.max);
					JSON_LOAD(attackColor_.range.end.min);
					JSON_LOAD(attackColor_.range.end.max);
					JSON_LOAD(defaultColor_.range.start.min);
					JSON_LOAD(defaultColor_.range.start.max);
					JSON_LOAD(defaultColor_.range.end.min);
					JSON_LOAD(defaultColor_.range.end.max);
					JSON_CLOSE();

				}
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	bossStateManager_->DrawImGui();
	bossHP_->DrawImGui();
	for (auto& colliderSizeName : colliderSize_) {
		if (colliderSizeName.second != 0.0f) {
			GPUParticleShaderStructs::Debug(colliderSizeName.first, emitters_[colliderSizeName.first]);
		}
	}
	GPUParticleShaderStructs::Debug("boss", meshEmitterDesc_);
	GPUParticleShaderStructs::Debug("boss", vertexEmitterDesc_);
#endif // _DEBUG
}

void Boss::DrawDebug(const ViewProjection& viewProjection) {

	for (auto& collider : bossCollider_) {
		collider.second->body->DrawCollision(viewProjection, collider.second->color);
		collider.second->attack->DrawCollision(viewProjection, collider.second->color);
	}
	animation_.DrawLine(animationTransform_);
}

void Boss::UpdateCollider() {

	for (auto& joint : animation_.skeleton.joints) {
		if (!joint.parent) {
			continue;
		}
		std::string jointName = EraseName(joint.name, "mixamorig:");
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * animationTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * animationTransform_.matWorld;

		Vector3 pos = MakeTranslateMatrix(worldMatrix);
		Vector3 parentPos = MakeTranslateMatrix(parentMatrix);

		//Vector3 born = (MakeTranslateMatrix(worldMatrix) - MakeTranslateMatrix(parentMatrix));
		//Vector3 center = MakeTranslateMatrix(parentMatrix) + born * 0.5f;
		//Quaternion orientation = MakeLookRotation(born.Normalized());
		//Vector3 size = { born.Length() * colliderSize_[joint.name], born.Length() * colliderSize_[joint.name],born.Length() };
		bossCollider_[jointName]->body->SetSegment(Segment(pos, parentPos));
		bossCollider_[jointName]->attack->SetSegment(Segment(pos, parentPos));
		bossCollider_[jointName]->body->SetRadius(colliderSize_[jointName]);
		bossCollider_[jointName]->attack->SetRadius(colliderSize_[jointName]);
		bossCollider_[jointName]->color = { 0.0f,1.0f,0.0f,1.0f };
	}
}

void Boss::UpdateGPUParticle() {
	for (auto& joint : animation_.skeleton.joints) {
		std::string jointName = EraseName(joint.name, "mixamorig:");
		if (!joint.parent.has_value() || colliderSize_[jointName] == 0.0f) {
			continue;
		}
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * worldTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * worldTransform_.matWorld;

		Vector3 worldPos = MakeTranslateMatrix(worldMatrix);
		Vector3 parentPos = MakeTranslateMatrix(parentMatrix);
		Vector3 born = (worldPos - parentPos);
		// 0
		{
			emitters_.at(jointName).emitterArea.capsule.segment.origin = worldPos;
			emitters_.at(jointName).emitterArea.capsule.segment.diff = parentPos;
			emitters_.at(jointName).emitterArea.capsule.radius = colliderSize_[jointName];
			emitters_.at(jointName).emitterArea.position = { 0.0f, 0.0f, 0.0f };
			emitters_.at(jointName).emitterArea.type = GPUParticleShaderStructs::Type::kCapsule;
			gpuParticleManager_->SetEmitter(emitters_.at(jointName));
		}
	}
}

void Boss::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	animationTransform_.UpdateMatrix();
}

void Boss::OnCollisionBody(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "PlayerBullet") {
		bossHP_->HitDamage(1);
	}
}

void Boss::OnCollisionAttack(const ColliderDesc& desc) {
	desc;
}
