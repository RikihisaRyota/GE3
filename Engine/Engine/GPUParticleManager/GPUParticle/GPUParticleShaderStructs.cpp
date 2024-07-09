#include "GPUParticleShaderStructs.h"

#include <unordered_map>

#include <optional>
#include <fstream>
#include <list>

#include "Engine/Json/JsonUtils.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Collision/CollisionAttribute.h"

int32_t GPUParticleShaderStructs::EmitterForCPU::staticEmitterCount = 0;
int32_t GPUParticleShaderStructs::FieldForCPU::staticFieldCount = 0;

namespace GPUParticleShaderStructs {

	void SaveMinMax(GPUParticleShaderStructs::UintMinMax& startEnd) {
		JSON_SAVE_BY_NAME("Min", startEnd.min);
		JSON_SAVE_BY_NAME("Max", startEnd.max);
	}

	void LoadMinMax(GPUParticleShaderStructs::UintMinMax& startEnd) {
		JSON_LOAD_BY_NAME("Min", startEnd.min);
		JSON_LOAD_BY_NAME("Max", startEnd.max);
	}

	void SaveMinMax(GPUParticleShaderStructs::FloatMinMax& startEnd) {
		JSON_SAVE_BY_NAME("Min", startEnd.min);
		JSON_SAVE_BY_NAME("Max", startEnd.max);
	}

	void LoadMinMax(GPUParticleShaderStructs::FloatMinMax& startEnd) {
		JSON_LOAD_BY_NAME("Min", startEnd.min);
		JSON_LOAD_BY_NAME("Max", startEnd.max);
	}

	void SaveMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd) {
		JSON_SAVE_BY_NAME("Min", startEnd.min);
		JSON_SAVE_BY_NAME("Max", startEnd.max);
	}

	void LoadMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd) {
		JSON_LOAD_BY_NAME("Min", startEnd.min);
		JSON_LOAD_BY_NAME("Max", startEnd.max);
	}

	void SaveMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd) {
		JSON_SAVE_BY_NAME("Min", startEnd.min);
		JSON_SAVE_BY_NAME("Max", startEnd.max);
	}

	void LoadMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd) {
		JSON_LOAD_BY_NAME("Min", startEnd.min);
		JSON_LOAD_BY_NAME("Max", startEnd.max);
	}

	void DrawMinMax(GPUParticleShaderStructs::UintMinMax& startEnd, float v_speed, int v_min, int v_max) {
#ifdef _DEBUG
		int min = static_cast<int>(startEnd.min);
		int max = static_cast<int>(startEnd.max);

		ImGui::DragInt("Min", &min, v_speed, v_min, max);
		ImGui::DragInt("Max", &max, v_speed, min, v_max);

		startEnd.min = static_cast<uint32_t>(min);
		startEnd.max = static_cast<uint32_t>(max);
#endif // _DEBUG
	}

	void DrawMinMax(GPUParticleShaderStructs::FloatMinMax& startEnd, float v_speed, float v_min, float v_max) {
#ifdef _DEBUG
		ImGui::DragFloat("Min", &startEnd.min, v_speed, v_min, v_max);
		ImGui::DragFloat("Max", &startEnd.max, v_speed, v_min, v_max);
#endif // _DEBUG
	}


	void DrawMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd, float v_speed, float v_min, float v_max) {
#ifdef _DEBUG
		ImGui::DragFloat3("Min", &startEnd.min.x, v_speed, v_min, v_max);
		ImGui::DragFloat3("Max", &startEnd.max.x, v_speed, v_min, v_max);
#endif // _DEBUG
	}

	void DrawMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd, float v_speed, float v_min, float v_max) {
#ifdef _DEBUG
		ImGui::DragFloat4("Min", &startEnd.min.x, v_speed, v_min, v_max);
		ImGui::DragFloat4("Max", &startEnd.max.x, v_speed, v_min, v_max);
#endif // _DEBUG
	}

	void DrawColorMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd) {
#ifdef _DEBUG
		ImGui::ColorEdit4("Min", &startEnd.min.x);
		ImGui::ColorEdit4("Max", &startEnd.max.x);
#endif // _DEBUG
	}

	void LoadStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd) {
		JSON_OBJECT("Start");
		LoadMinMax(startEnd.start);
		JSON_PARENT();
		JSON_OBJECT("End");
		LoadMinMax(startEnd.end);
		JSON_PARENT();
	}

	void SaveStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd) {
		JSON_OBJECT("Start");
		SaveMinMax(startEnd.start);
		JSON_PARENT();
		JSON_OBJECT("End");
		SaveMinMax(startEnd.end);
		JSON_PARENT();
	}

	void LoadStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd) {
		JSON_OBJECT("Start");
		LoadMinMax(startEnd.start);
		JSON_PARENT();
		JSON_OBJECT("End");
		LoadMinMax(startEnd.end);
		JSON_PARENT();
	}

	void SaveStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd) {
		JSON_OBJECT("Start");
		SaveMinMax(startEnd.start);
		JSON_PARENT();
		JSON_OBJECT("End");
		SaveMinMax(startEnd.end);
		JSON_PARENT();
	}


	void DrawStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd, float v_speed, float v_min, float v_max) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Start")) {
			DrawMinMax(startEnd.start, v_speed, v_min, v_max);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("End")) {
			DrawMinMax(startEnd.end, v_speed, v_min, v_max);
			ImGui::TreePop();
		}
#endif // _DEBUG
	}
	void DrawStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd, float v_speed, float v_min, float v_max) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Start")) {
			DrawMinMax(startEnd.start, v_speed, v_min, v_max);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("End")) {
			DrawMinMax(startEnd.end, v_speed, v_min, v_max);
			ImGui::TreePop();
		}
#endif // _DEBUG
	}
	void DrawColor(GPUParticleShaderStructs::Vector4StartEnd& startEnd) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Start")) {
			DrawColorMinMax(startEnd.start);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("End")) {
			DrawColorMinMax(startEnd.end);
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	std::unordered_map<std::string, std::tuple<bool*, EmitterForCPU*>>debugEmitters_;
	std::unordered_map<std::string, std::tuple<bool*, MeshEmitterDesc*>>debugMeshEmitterDesc_;
	std::unordered_map<std::string, std::tuple<bool*, VertexEmitterDesc*>>debugVertexEmitterDesc_;
}

void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::EmitterForCPU*>e) {
#ifdef _DEBUG
	ImGui::Begin(("Emitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto emitter = std::get<1>(e);
	if (ImGui::Button("Delete")) {
		*std::get<0>(e) = false;
	}
	ImGui::Text("EmitterCount : %d", emitter->emitterCount);
	if (ImGui::TreeNode("Area")) {
		switch (emitter->emitterArea.type) {
		case GPUParticleShaderStructs::kAABB:
			if (ImGui::TreeNode("AABB")) {
				DrawMinMax(emitter->emitterArea.aabb.area);
				ImGui::DragFloat3("Position", &emitter->emitterArea.aabb.position.x, 0.1f);
				ImGui::TreePop();
			}
			break;
		case GPUParticleShaderStructs::kSphere:
			if (ImGui::TreeNode("Sphere")) {
				ImGui::DragFloat("Radius", &emitter->emitterArea.sphere.radius, 0.1f);
				ImGui::DragFloat3("Position", &emitter->emitterArea.sphere.position.x, 0.1f);
				DrawMinMax(emitter->emitterArea.sphere.distanceFactor, 0.01f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			break;
		case GPUParticleShaderStructs::kCapsule:
			if (ImGui::TreeNode("Capsule")) {
				ImGui::DragFloat3("Start", &emitter->emitterArea.capsule.segment.origin.x, 0.1f);
				ImGui::DragFloat3("End", &emitter->emitterArea.capsule.segment.diff.x, 0.1f);
				ImGui::DragFloat("Radius", &emitter->emitterArea.capsule.radius, 0.1f);
				DrawMinMax(emitter->emitterArea.capsule.distanceFactor, 0.01f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			break;
		case GPUParticleShaderStructs::kFigureCount:
			break;
		default:
			break;
		}

		std::vector<const char*> stateNamesCStr{ "AABB","Sphere","Capsule" };
		int currentState = static_cast<int>(emitter->emitterArea.type);

		// ステートを変更するImGui::Comboの作成
		if (ImGui::Combo("Type", &currentState, stateNamesCStr.data(), int(stateNamesCStr.size()))) {
			emitter->emitterArea.type = static_cast<Type>(currentState);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Scale")) {
		if (!emitter->scale.isSame) {
			DrawStartEnd(emitter->scale.range, 0.01f, 0.0f);
		}
		else {
			if (ImGui::TreeNode("Start")) {
				ImGui::DragFloat("Min", &emitter->scale.range.start.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &emitter->scale.range.start.max.x, 0.01f, 0.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("End")) {
				ImGui::DragFloat("Min", &emitter->scale.range.end.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &emitter->scale.range.end.max.x, 0.01f, 0.0f);
				ImGui::TreePop();
			}
			emitter->scale.range.start.min.y = emitter->scale.range.start.min.x;
			emitter->scale.range.start.min.z = emitter->scale.range.start.min.x;
			emitter->scale.range.start.max.y = emitter->scale.range.start.max.x;
			emitter->scale.range.start.max.z = emitter->scale.range.start.max.x;
			emitter->scale.range.end.min.y = emitter->scale.range.end.min.x;
			emitter->scale.range.end.min.z = emitter->scale.range.end.min.x;
			emitter->scale.range.end.max.y = emitter->scale.range.end.max.x;
			emitter->scale.range.end.max.z = emitter->scale.range.end.max.x;
		}
		ImGui::Checkbox("IsSame", reinterpret_cast<bool*>(&emitter->scale.isSame));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Rotate")) {
		if (ImGui::TreeNode("InitializeAngle")) {
			DrawMinMax(emitter->rotate.initializeAngle);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("RotateSpeed")) {
			DrawMinMax(emitter->rotate.rotateSpeed);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Velocity3D")) {
		DrawMinMax(emitter->velocity.range, 0.1f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Color")) {
		DrawColor(emitter->color.range);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Frequency")) {
		ImGui::DragInt("Interval", reinterpret_cast<int*>(&emitter->frequency.interval), 1, 0);
		if (!emitter->frequency.isLoop) {
			ImGui::DragInt("EmitterLifeTime", reinterpret_cast<int*>(&emitter->frequency.emitterLife), 1, 0);
		}
		ImGui::Checkbox("IsLoop", reinterpret_cast<bool*>(&emitter->frequency.isLoop));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("ParticleLife")) {
		DrawMinMax(emitter->particleLifeSpan.range);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("TextureHandle")) {
		std::list<std::string> stageList;
		for (int i = 0; i < TextureManager::GetInstance()->GetTextureSize(); i++) {
			stageList.emplace_back(TextureManager::GetInstance()->GetTexture(i).GetName().string());

		}
		// std::vector に変換する
		std::vector<const char*> stageArray;
		for (const auto& stage : stageList) {
			stageArray.push_back(stage.c_str());
		}

		int currentTexture = TextureManager::GetInstance()->GetTextureLocation(emitter->textureIndex);
		// Combo を使用する
		if (ImGui::Combo("Texture", &currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
			emitter->textureIndex = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("CreateParticle")) {
		ImGui::DragInt("Num", reinterpret_cast<int*>(&emitter->createParticleNum));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("CollisionInfo")) {
		if (ImGui::TreeNode("Attribute")) {
			ImGui::Text("Collision Attribute:");
			ImGui::CheckboxFlags("Player", &emitter->collisionInfo.attribute, CollisionAttribute::Player);
			ImGui::CheckboxFlags("Player Bullet", &emitter->collisionInfo.attribute, CollisionAttribute::PlayerBullet);
			ImGui::CheckboxFlags("Boss Body", &emitter->collisionInfo.attribute, CollisionAttribute::BossBody);
			ImGui::CheckboxFlags("Boss Attack", &emitter->collisionInfo.attribute, CollisionAttribute::BossAttack);
			ImGui::CheckboxFlags("GameObject", &emitter->collisionInfo.attribute, CollisionAttribute::GameObject);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Mask")) {
			ImGui::Text("Collision Mask:");
			ImGui::CheckboxFlags("Player", &emitter->collisionInfo.mask, CollisionAttribute::Player);
			ImGui::CheckboxFlags("Player Bullet", &emitter->collisionInfo.mask, CollisionAttribute::PlayerBullet);
			ImGui::CheckboxFlags("Boss Body", &emitter->collisionInfo.mask, CollisionAttribute::BossBody);
			ImGui::CheckboxFlags("Boss Attack", &emitter->collisionInfo.mask, CollisionAttribute::BossAttack);
			ImGui::CheckboxFlags("GameObject", &emitter->collisionInfo.mask, CollisionAttribute::GameObject);

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *emitter);
	}
	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}

void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::MeshEmitterDesc*> d) {
#ifdef _DEBUG
	ImGui::Begin(("MeshEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto& desc = std::get<1>(d);
	if (ImGui::Button("Delete")) {
		*std::get<0>(d) = false;
	}
	if (ImGui::TreeNode("Scale")) {
		if (!desc->emitter.scale.isSame) {
			DrawStartEnd(desc->emitter.scale.range, 0.01f, 0.0f);
		}
		else {
			if (ImGui::TreeNode("Start")) {
				ImGui::DragFloat("Min", &desc->emitter.scale.range.start.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &desc->emitter.scale.range.start.max.x, 0.01f, 0.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("End")) {
				ImGui::DragFloat("Min", &desc->emitter.scale.range.end.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &desc->emitter.scale.range.end.max.x, 0.01f, 0.0f);
				ImGui::TreePop();
			}
			desc->emitter.scale.range.start.min.y = desc->emitter.scale.range.start.min.x;
			desc->emitter.scale.range.start.min.z = desc->emitter.scale.range.start.min.x;
			desc->emitter.scale.range.start.max.y = desc->emitter.scale.range.start.max.x;
			desc->emitter.scale.range.start.max.z = desc->emitter.scale.range.start.max.x;
			desc->emitter.scale.range.end.min.y = desc->emitter.scale.range.end.min.x;
			desc->emitter.scale.range.end.min.z = desc->emitter.scale.range.end.min.x;
			desc->emitter.scale.range.end.max.y = desc->emitter.scale.range.end.max.x;
			desc->emitter.scale.range.end.max.z = desc->emitter.scale.range.end.max.x;
		}
		ImGui::Checkbox("IsSame", reinterpret_cast<bool*>(&desc->emitter.scale.isSame));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Rotate")) {
		if (ImGui::TreeNode("InitializeAngle")) {
			DrawMinMax(desc->emitter.rotate.initializeAngle);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("RotateSpeed")) {
			DrawMinMax(desc->emitter.rotate.rotateSpeed);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Velocity3D")) {
		DrawMinMax(desc->emitter.velocity.range, 0.1f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Color")) {
		DrawColor(desc->emitter.color.range);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("ParticleLife")) {
		DrawMinMax(desc->emitter.particleLifeSpan.range, 1.0f, 0);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("TextureHandle")) {
		std::list<std::string> stageList;
		for (int i = 0; i < TextureManager::GetInstance()->GetTextureSize(); i++) {
			stageList.emplace_back(TextureManager::GetInstance()->GetTexture(i).GetName().string());

		}
		// std::vector に変換する
		std::vector<const char*> stageArray;
		for (const auto& stage : stageList) {
			stageArray.push_back(stage.c_str());
		}

		int currentTexture = TextureManager::GetInstance()->GetTextureLocation(desc->emitter.textureIndex);
		// Combo を使用する
		if (ImGui::Combo("Texture", &currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
			desc->emitter.textureIndex = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("NumCreate")) {
		int num = desc->numCreate;
		ImGui::DragInt("num", &num, 1.0f, 0, 50);
		desc->numCreate = num;
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("CollisionInfo")) {
		if (ImGui::TreeNode("Attribute")) {
			ImGui::Text("Collision Attribute:");
			ImGui::CheckboxFlags("Player", &desc->emitter.collisionInfo.attribute, CollisionAttribute::Player);
			ImGui::CheckboxFlags("Player Bullet", &desc->emitter.collisionInfo.attribute, CollisionAttribute::PlayerBullet);
			ImGui::CheckboxFlags("Boss Body", &desc->emitter.collisionInfo.attribute, CollisionAttribute::BossBody);
			ImGui::CheckboxFlags("Boss Attack", &desc->emitter.collisionInfo.attribute, CollisionAttribute::BossAttack);
			ImGui::CheckboxFlags("GameObject", &desc->emitter.collisionInfo.attribute, CollisionAttribute::GameObject);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Mask")) {
			ImGui::Text("Collision Mask:");
			ImGui::CheckboxFlags("Player", &desc->emitter.collisionInfo.mask, CollisionAttribute::Player);
			ImGui::CheckboxFlags("Player Bullet", &desc->emitter.collisionInfo.mask, CollisionAttribute::PlayerBullet);
			ImGui::CheckboxFlags("Boss Body", &desc->emitter.collisionInfo.mask, CollisionAttribute::BossBody);
			ImGui::CheckboxFlags("Boss Attack", &desc->emitter.collisionInfo.mask, CollisionAttribute::BossAttack);
			ImGui::CheckboxFlags("GameObject", &desc->emitter.collisionInfo.mask, CollisionAttribute::GameObject);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *desc);
	}

	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}

void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::VertexEmitterDesc*> d) {
#ifdef _DEBUG
	ImGui::Begin(("VertexEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto& desc = std::get<1>(d);
	if (ImGui::Button("Delete")) {
		*std::get<0>(d) = false;
	}
	if (ImGui::TreeNode("Scale")) {
		if (!desc->emitter.scale.isSame) {
			DrawStartEnd(desc->emitter.scale.range, 0.01f, 0.0f);
		}
		else {
			if (ImGui::TreeNode("Start")) {
				ImGui::DragFloat("Min", &desc->emitter.scale.range.start.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &desc->emitter.scale.range.start.max.x, 0.01f, 0.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("End")) {
				ImGui::DragFloat("Min", &desc->emitter.scale.range.end.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &desc->emitter.scale.range.end.max.x, 0.01f, 0.0f);

				ImGui::TreePop();
			}
			desc->emitter.scale.range.start.min.y = desc->emitter.scale.range.start.min.x;
			desc->emitter.scale.range.start.min.z = desc->emitter.scale.range.start.min.x;
			desc->emitter.scale.range.start.max.y = desc->emitter.scale.range.start.max.x;
			desc->emitter.scale.range.start.max.z = desc->emitter.scale.range.start.max.x;
			desc->emitter.scale.range.end.min.y = desc->emitter.scale.range.end.min.x;
			desc->emitter.scale.range.end.min.z = desc->emitter.scale.range.end.min.x;
			desc->emitter.scale.range.end.max.y = desc->emitter.scale.range.end.max.x;
			desc->emitter.scale.range.end.max.z = desc->emitter.scale.range.end.max.x;
		}
		ImGui::Checkbox("IsSame", reinterpret_cast<bool*>(&desc->emitter.scale.isSame));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Rotate")) {
		if (ImGui::TreeNode("InitializeAngle")) {
			DrawMinMax(desc->emitter.rotate.initializeAngle);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("RotateSpeed")) {
			DrawMinMax(desc->emitter.rotate.rotateSpeed);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Velocity3D")) {
		DrawMinMax(desc->emitter.velocity.range, 0.1f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Color")) {
		DrawColor(desc->emitter.color.range);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("ParticleLife")) {
		DrawMinMax(desc->emitter.particleLifeSpan.range, 1.0f, 0);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("TextureHandle")) {
		std::list<std::string> stageList;
		for (int i = 0; i < TextureManager::GetInstance()->GetTextureSize(); i++) {
			stageList.emplace_back(TextureManager::GetInstance()->GetTexture(i).GetName().string());

		}
		// std::vector に変換する
		std::vector<const char*> stageArray;
		for (const auto& stage : stageList) {
			stageArray.push_back(stage.c_str());
		}

		int currentTexture = TextureManager::GetInstance()->GetTextureLocation(desc->emitter.textureIndex);
		// Combo を使用する
		if (ImGui::Combo("Texture", &currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
			desc->emitter.textureIndex = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("CollisionInfo")) {
		if (ImGui::TreeNode("Attribute")) {
			ImGui::Text("Collision Attribute:");
			ImGui::CheckboxFlags("Player", &desc->emitter.collisionInfo.attribute, CollisionAttribute::Player);
			ImGui::CheckboxFlags("Player Bullet", &desc->emitter.collisionInfo.attribute, CollisionAttribute::PlayerBullet);
			ImGui::CheckboxFlags("Boss Body", &desc->emitter.collisionInfo.attribute, CollisionAttribute::BossBody);
			ImGui::CheckboxFlags("Boss Attack", &desc->emitter.collisionInfo.attribute, CollisionAttribute::BossAttack);
			ImGui::CheckboxFlags("GameObject", &desc->emitter.collisionInfo.attribute, CollisionAttribute::GameObject);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Mask")) {
			ImGui::Text("Collision Mask:");
			ImGui::CheckboxFlags("Player", &desc->emitter.collisionInfo.mask, CollisionAttribute::Player);
			ImGui::CheckboxFlags("Player Bullet", &desc->emitter.collisionInfo.mask, CollisionAttribute::PlayerBullet);
			ImGui::CheckboxFlags("Boss Body", &desc->emitter.collisionInfo.mask, CollisionAttribute::BossBody);
			ImGui::CheckboxFlags("Boss Attack", &desc->emitter.collisionInfo.mask, CollisionAttribute::BossAttack);
			ImGui::CheckboxFlags("GameObject", &desc->emitter.collisionInfo.mask, CollisionAttribute::GameObject);

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *desc);
	}

	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	if (debugEmitters_.find(name) == debugEmitters_.end()) {
		bool* falseFlag = new bool(false);
		debugEmitters_[name] = std::make_tuple(falseFlag, &emitter);
	}
}

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& emitter) {
	if (debugMeshEmitterDesc_.find(name) == debugMeshEmitterDesc_.end()) {
		bool* falseFlag = new bool(false);
		debugMeshEmitterDesc_[name] = std::make_tuple(falseFlag, &emitter);
	}
}


void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& emitter) {
	if (debugVertexEmitterDesc_.find(name) == debugVertexEmitterDesc_.end()) {
		bool* falseFlag = new bool(false);
		debugVertexEmitterDesc_[name] = std::make_tuple(falseFlag, &emitter);
	}
}


void GPUParticleShaderStructs::Update() {
#ifdef _DEBUG
	ImGui::Begin("GPUParticle");
	if (ImGui::BeginMenu("Emitter")) {
		for (auto& emitter : debugEmitters_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("MeshParticle")) {
		for (auto& emitter : debugMeshEmitterDesc_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("VertexParticle")) {
		for (auto& emitter : debugVertexEmitterDesc_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}
	ImGui::End();

	for (auto it = debugEmitters_.begin(); it != debugEmitters_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugEmitters_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
	for (auto it = debugMeshEmitterDesc_.begin(); it != debugMeshEmitterDesc_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugMeshEmitterDesc_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
	for (auto it = debugVertexEmitterDesc_.begin(); it != debugVertexEmitterDesc_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugVertexEmitterDesc_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
#endif // _DEBUG
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/Emitter/" + name + ".json");

	JSON_OBJECT("EmitterArea");
	JSON_LOAD_BY_NAME("type", emitter.emitterArea.type);
	JSON_OBJECT("EmitterAABB");
	LoadMinMax(emitter.emitterArea.aabb.area);
	JSON_LOAD_BY_NAME("position", emitter.emitterArea.aabb.position);
	JSON_PARENT();
	JSON_OBJECT("EmitterSphere");
	JSON_LOAD_BY_NAME("position", emitter.emitterArea.sphere.position);
	JSON_LOAD_BY_NAME("radius", emitter.emitterArea.sphere.radius);
	JSON_OBJECT("distanceFactor");
	LoadMinMax(emitter.emitterArea.sphere.distanceFactor);
	JSON_PARENT();
	JSON_PARENT();

	JSON_OBJECT("EmitterCapsule");
	JSON_OBJECT("EmitterSegment");
	JSON_LOAD_BY_NAME("start", emitter.emitterArea.capsule.segment.origin);
	JSON_LOAD_BY_NAME("end", emitter.emitterArea.capsule.segment.diff);
	JSON_PARENT();
	JSON_LOAD_BY_NAME("radius", emitter.emitterArea.capsule.radius);
	JSON_OBJECT("distanceFactor");
	LoadMinMax(emitter.emitterArea.capsule.distanceFactor);
	JSON_PARENT();
	JSON_PARENT();

	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	LoadStartEnd(emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterFrequency");
	JSON_LOAD_BY_NAME("emitterLife", emitter.frequency.emitterLife);
	JSON_LOAD_BY_NAME("interval", emitter.frequency.interval);
	JSON_LOAD_BY_NAME("isLoop", emitter.frequency.isLoop);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeSpan");
	LoadMinMax(emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_OBJECT("rotateSpeed");
	LoadMinMax(emitter.rotate.rotateSpeed);
	JSON_PARENT();
	JSON_OBJECT("initializeAngle");
	LoadMinMax(emitter.rotate.initializeAngle);
	JSON_ROOT();

	JSON_OBJECT("ScaleAnimation");
	LoadStartEnd(emitter.scale.range);
	JSON_LOAD_BY_NAME("isSame", emitter.scale.isSame);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	LoadMinMax(emitter.velocity.range);
	JSON_ROOT();

	JSON_OBJECT("CollisionInfo");
	JSON_LOAD_BY_NAME("mask", emitter.collisionInfo.mask);
	JSON_LOAD_BY_NAME("attribute", emitter.collisionInfo.attribute);
	JSON_ROOT();

	JSON_LOAD_BY_NAME("createParticleNum", emitter.createParticleNum);
	std::string path{};
	JSON_LOAD_BY_NAME("textureIndex", path);
	if (!path.empty()) {
		emitter.textureIndex = TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->Load(path)).GetDescriptorIndex();
	}

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/Emitter/" + name + ".json");

	JSON_OBJECT("EmitterArea");
	JSON_SAVE_BY_NAME("type", emitter.emitterArea.type);
	JSON_OBJECT("EmitterAABB");
	SaveMinMax(emitter.emitterArea.aabb.area);
	JSON_SAVE_BY_NAME("position", emitter.emitterArea.aabb.position);
	JSON_PARENT();
	JSON_OBJECT("EmitterSphere");
	JSON_SAVE_BY_NAME("radius", emitter.emitterArea.sphere.radius);
	JSON_SAVE_BY_NAME("position", emitter.emitterArea.sphere.position);
	JSON_OBJECT("distanceFactor");
	SaveMinMax(emitter.emitterArea.sphere.distanceFactor);
	JSON_PARENT();
	JSON_PARENT();


	JSON_OBJECT("EmitterCapsule");
	JSON_OBJECT("EmitterSegment");
	JSON_SAVE_BY_NAME("start", emitter.emitterArea.capsule.segment.origin);
	JSON_SAVE_BY_NAME("end", emitter.emitterArea.capsule.segment.diff);
	JSON_PARENT();
	JSON_SAVE_BY_NAME("radius", emitter.emitterArea.capsule.radius);
	JSON_OBJECT("distanceFactor");
	SaveMinMax(emitter.emitterArea.capsule.distanceFactor);
	JSON_PARENT();
	JSON_PARENT();

	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	SaveStartEnd(emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterFrequency");
	JSON_SAVE_BY_NAME("emitterLife", emitter.frequency.emitterLife);
	JSON_SAVE_BY_NAME("interval", emitter.frequency.interval);
	JSON_SAVE_BY_NAME("isLoop", emitter.frequency.isLoop);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeSpan");
	SaveMinMax(emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_OBJECT("rotateSpeed");
	SaveMinMax(emitter.rotate.rotateSpeed);
	JSON_PARENT();
	JSON_OBJECT("initializeAngle");
	SaveMinMax(emitter.rotate.initializeAngle);
	JSON_ROOT();

	JSON_OBJECT("ScaleAnimation");
	SaveStartEnd(emitter.scale.range);
	JSON_SAVE_BY_NAME("isSame", emitter.scale.isSame);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	SaveMinMax(emitter.velocity.range);
	JSON_ROOT();

	JSON_SAVE_BY_NAME("createParticleNum", emitter.createParticleNum);
	JSON_SAVE_BY_NAME("textureIndex", TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->GetTextureLocation(emitter.textureIndex)).GetPath().string());

	JSON_OBJECT("CollisionInfo");
	JSON_SAVE_BY_NAME("mask", emitter.collisionInfo.mask);
	JSON_SAVE_BY_NAME("attribute", emitter.collisionInfo.attribute);
	JSON_ROOT();

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	LoadStartEnd(desc.emitter.scale.range);
	JSON_LOAD_BY_NAME("isSame", desc.emitter.scale.isSame);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	LoadMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	LoadStartEnd(desc.emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_OBJECT("rotateSpeed");
	LoadMinMax(desc.emitter.rotate.rotateSpeed);
	JSON_PARENT();
	JSON_OBJECT("initializeAngle");
	LoadMinMax(desc.emitter.rotate.initializeAngle);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	LoadMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_LOAD_BY_NAME("NumCreate", desc.numCreate);
	std::string path{};
	JSON_LOAD_BY_NAME("textureIndex", path);
	if (!path.empty()) {
		desc.emitter.textureIndex = TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->Load(path)).GetDescriptorIndex();
	}

	JSON_OBJECT("CollisionInfo");
	JSON_LOAD_BY_NAME("mask", desc.emitter.collisionInfo.mask);
	JSON_LOAD_BY_NAME("attribute", desc.emitter.collisionInfo.attribute);
	JSON_ROOT();

	JSON_CLOSE();
}



void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	SaveStartEnd(desc.emitter.scale.range);
	JSON_SAVE_BY_NAME("isSame", desc.emitter.scale.isSame);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	SaveMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	SaveStartEnd(desc.emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_OBJECT("rotateSpeed");
	SaveMinMax(desc.emitter.rotate.rotateSpeed);
	JSON_PARENT();
	JSON_OBJECT("initializeAngle");
	SaveMinMax(desc.emitter.rotate.initializeAngle);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	SaveMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_SAVE_BY_NAME("NumCreate", desc.numCreate);

	JSON_SAVE_BY_NAME("textureIndex", TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->GetTextureLocation(desc.emitter.textureIndex)).GetPath().string());

	JSON_OBJECT("CollisionInfo");
	JSON_SAVE_BY_NAME("mask", desc.emitter.collisionInfo.mask);
	JSON_SAVE_BY_NAME("attribute", desc.emitter.collisionInfo.attribute);
	JSON_ROOT();

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	LoadStartEnd(desc.emitter.scale.range);
	JSON_LOAD_BY_NAME("isSame", desc.emitter.scale.isSame);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	LoadMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	LoadStartEnd(desc.emitter.color.range);
	JSON_ROOT();


	JSON_OBJECT("RotateAnimation");
	JSON_OBJECT("rotateSpeed");
	LoadMinMax(desc.emitter.rotate.rotateSpeed);
	JSON_PARENT();
	JSON_OBJECT("initializeAngle");
	LoadMinMax(desc.emitter.rotate.initializeAngle);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	LoadMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	std::string path{};
	JSON_LOAD_BY_NAME("textureIndex", path);
	if (!path.empty()) {
		desc.emitter.textureIndex = TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->Load(path)).GetDescriptorIndex();
	}
	JSON_OBJECT("CollisionInfo");
	JSON_LOAD_BY_NAME("mask", desc.emitter.collisionInfo.mask);
	JSON_LOAD_BY_NAME("attribute", desc.emitter.collisionInfo.attribute);
	JSON_ROOT();

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	SaveStartEnd(desc.emitter.scale.range);
	JSON_SAVE_BY_NAME("isSame", desc.emitter.scale.isSame);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	SaveMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	SaveStartEnd(desc.emitter.color.range);
	JSON_ROOT();


	JSON_OBJECT("RotateAnimation");
	JSON_OBJECT("rotateSpeed");
	SaveMinMax(desc.emitter.rotate.rotateSpeed);
	JSON_PARENT();
	JSON_OBJECT("initializeAngle");
	SaveMinMax(desc.emitter.rotate.initializeAngle);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	SaveMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_SAVE_BY_NAME("textureIndex", TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->GetTextureLocation(desc.emitter.textureIndex)).GetPath().string());

	JSON_OBJECT("CollisionInfo");
	JSON_SAVE_BY_NAME("mask", desc.emitter.collisionInfo.mask);
	JSON_SAVE_BY_NAME("attribute", desc.emitter.collisionInfo.attribute);
	JSON_ROOT();

	JSON_CLOSE();
}

GPUParticleShaderStructs::MeshEmitterDesc::MeshEmitterDesc() {
	buffer.Create(L"MeshEmitterBuffer", sizeof(MeshEmitter));
}

GPUParticleShaderStructs::VertexEmitterDesc::VertexEmitterDesc() {
	buffer.Create(L"VertexEmitterBuffer", sizeof(VertexEmitterDesc));
}
