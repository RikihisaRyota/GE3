#include "GPUParticleShaderStructs.h"

#include <map>

#include <optional>
#include <fstream>
#include <list>

#include "Engine/Json/JsonUtils.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/DrawLine/DrawLine.h"
#include "Engine/Math/MyMath.h"

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

	void DrawArea(GPUParticleShaderStructs::EmitterArea& area) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Area")) {
			switch (area.type) {
			case GPUParticleShaderStructs::kAABB:
				if (ImGui::TreeNode("AABB")) {
					DrawMinMax(area.aabb.area);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kSphere:
				if (ImGui::TreeNode("Sphere")) {
					ImGui::DragFloat("Radius", &area.sphere.radius, 0.1f);
					DrawMinMax(area.sphere.distanceFactor, 0.01f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kCapsule:
				if (ImGui::TreeNode("Capsule")) {
					ImGui::DragFloat3("Start", &area.capsule.segment.origin.x, 0.1f);
					ImGui::DragFloat3("End", &area.capsule.segment.diff.x, 0.1f);
					ImGui::DragFloat("Radius", &area.capsule.radius, 0.1f);
					DrawMinMax(area.capsule.distanceFactor, 0.01f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kFigureCount:
				break;
			default:
				break;
			}
			ImGui::DragFloat3("Position", &area.position.x, 0.1f);

			std::vector<const char*> stateNamesCStr{ "AABB","Sphere","Capsule" };
			int currentState = static_cast<int>(area.type);

			// ステートを変更するImGui::Comboの作成
			if (ImGui::Combo("Type", &currentState, stateNamesCStr.data(), int(stateNamesCStr.size()))) {
				area.type = static_cast<Type>(currentState);
			}
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawScale(GPUParticleShaderStructs::ScaleAnimation& scale) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Scale")) {
			if (!scale.isStaticSize && !scale.isUniformScale) {
				DrawStartEnd(scale.range);
			}
			else if (!scale.isStaticSize && scale.isUniformScale) {
				if (ImGui::TreeNode("Start")) {
					ImGui::DragFloat("Min", &scale.range.start.min.x, 0.01f, 0.0f);
					ImGui::DragFloat("Max", &scale.range.start.max.x, 0.01f, 0.0f);
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("End")) {
					ImGui::DragFloat("Min", &scale.range.end.min.x, 0.01f, 0.0f);
					ImGui::DragFloat("Max", &scale.range.end.max.x, 0.01f, 0.0f);
					ImGui::TreePop();
				}
				scale.range.start.min.y = scale.range.start.min.x;
				scale.range.start.min.z = scale.range.start.min.x;
				scale.range.start.max.y = scale.range.start.max.x;
				scale.range.start.max.z = scale.range.start.max.x;
				scale.range.end.min.y = scale.range.end.min.x;
				scale.range.end.min.z = scale.range.end.min.x;
				scale.range.end.max.y = scale.range.end.max.x;
				scale.range.end.max.z = scale.range.end.max.x;
			}
			else if (scale.isStaticSize && !scale.isUniformScale) {
				ImGui::DragFloat3("Min", &scale.range.start.min.x, 0.01f, 0.0f);
				ImGui::DragFloat3("Max", &scale.range.start.max.x, 0.01f, 0.0f);
				scale.range.end = scale.range.start;
			}
			else {
				ImGui::DragFloat("Min", &scale.range.start.min.x, 0.01f, 0.0f);
				ImGui::DragFloat("Max", &scale.range.start.max.x, 0.01f, 0.0f);
				scale.range.start.min.y = scale.range.start.min.x;
				scale.range.start.min.z = scale.range.start.min.x;
				scale.range.start.max.y = scale.range.start.max.x;
				scale.range.start.max.z = scale.range.start.max.x;
				scale.range.end = scale.range.start;
			}
			ImGui::Checkbox("IsUniformScale", reinterpret_cast<bool*>(&scale.isUniformScale));
			ImGui::Checkbox("IsStaticSize", reinterpret_cast<bool*>(&scale.isStaticSize));
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawRotate(GPUParticleShaderStructs::RotateAnimation& rotate) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Rotate")) {
			if (ImGui::TreeNode("InitializeAngle")) {
				DrawMinMax(rotate.initializeAngle);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("RotateSpeed")) {
				DrawMinMax(rotate.rotateSpeed);
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawVelocity(GPUParticleShaderStructs::Velocity3D& velocity) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Velocity3D")) {
			DrawMinMax(velocity.range, 0.1f);
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawColor(GPUParticleShaderStructs::EmitterColor& color) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Color")) {
			if (color.isStaticColor) {
				DrawColorMinMax(color.range.start);
				color.range.end = color.range.start;
			}
			else {
				DrawColor(color.range);
			}
			ImGui::Checkbox("IsStaticColor", reinterpret_cast<bool*>(&color.isStaticColor));
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Frequency")) {
			ImGui::DragInt("Interval", reinterpret_cast<int*>(&frequency.interval), 1, 0);
			if (!frequency.isLoop) {
				ImGui::DragInt("EmitterLifeTime", reinterpret_cast<int*>(&frequency.emitterLife), 1, 0);
			}
			ImGui::Checkbox("IsLoop", reinterpret_cast<bool*>(&frequency.isLoop));
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan) {
#ifdef _DEBUG
		if (ImGui::TreeNode("ParticleLife")) {
			DrawMinMax(particleLifeSpan.range);
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawTextureHandle(uint32_t& texture) {
#ifdef _DEBUG
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

			int currentTexture = TextureManager::GetInstance()->GetTextureLocation(texture);
			// Combo を使用する
			if (ImGui::Combo("Texture", &currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
				texture = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
			}

			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawCreateParticleNum(uint32_t& createParticleNum) {
#ifdef _DEBUG
		if (ImGui::TreeNode("CreateParticle")) {
			ImGui::DragInt("Num", reinterpret_cast<int*>(&createParticleNum));
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawParent(uint32_t& parent) {
		bool isParent = static_cast<bool>(parent);

		if (ImGui::Checkbox("IsParent", &isParent)) {
			parent = static_cast<uint32_t>(isParent);
		}
	}

	void DrawCollisionInfo(GPUParticleShaderStructs::ParticleAttributes& particleAttributes) {
#ifdef _DEBUG
		if (ImGui::TreeNode("CollisionInfo")) {
			if (ImGui::TreeNode("Attribute")) {
				ImGui::Text("Collision Attribute:");
				ImGui::CheckboxFlags("Player", &particleAttributes.attribute, CollisionAttribute::Player);
				ImGui::CheckboxFlags("Player Bullet", &particleAttributes.attribute, CollisionAttribute::PlayerBullet);
				ImGui::CheckboxFlags("Boss", &particleAttributes.attribute, CollisionAttribute::Boss);
				ImGui::CheckboxFlags("GameObject", &particleAttributes.attribute, CollisionAttribute::GameObject);
				ImGui::CheckboxFlags("ParticleField", &particleAttributes.attribute, CollisionAttribute::ParticleField);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Mask")) {
				ImGui::Text("Collision Mask:");
				ImGui::CheckboxFlags("Player", &particleAttributes.mask, CollisionAttribute::Player);
				ImGui::CheckboxFlags("Player Bullet", &particleAttributes.mask, CollisionAttribute::PlayerBullet);
				ImGui::CheckboxFlags("Boss", &particleAttributes.mask, CollisionAttribute::Boss);
				ImGui::CheckboxFlags("GameObject", &particleAttributes.mask, CollisionAttribute::GameObject);

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawField(GPUParticleShaderStructs::Field& field) {
#ifdef _DEBUG
		if (ImGui::TreeNode("FieldInfo")) {
			switch (field.type) {
			case GPUParticleShaderStructs::kAttraction:
				if (ImGui::TreeNode("Attraction")) {
					ImGui::DragFloat("Attraction", &field.attraction.attraction, 0.01f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kExternalForce:
				if (ImGui::TreeNode("ExternalForce")) {
					ImGui::DragFloat3("ExternalForce", &field.externalForce.externalForce.x, 0.01f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kFieldCount:
				break;
			default:
				break;
			}
			std::vector<const char*> typeCStr{ "Attraction","ExternalForce" };
			int currentType = static_cast<int>(field.type);

			// ステートを変更するImGui::Comboの作成
			if (ImGui::Combo("Type", &currentType, typeCStr.data(), int(typeCStr.size()))) {
				field.type = static_cast<Type>(currentType);
			}
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawFieldFrequency(GPUParticleShaderStructs::FieldFrequency& fieldFrequency) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Frequency")) {
			ImGui::Checkbox("IsLoop", reinterpret_cast<bool*>(&fieldFrequency.isLoop));
			if (!fieldFrequency.isLoop) {
				ImGui::DragInt("LifeCount", reinterpret_cast<int*>(&fieldFrequency.lifeCount), 1, 0);
			}
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void LoadArea(GPUParticleShaderStructs::EmitterArea& area) {
		JSON_OBJECT("EmitterArea");
		JSON_LOAD_BY_NAME("position", area.position);
		JSON_LOAD_BY_NAME("type", area.type);
		JSON_OBJECT("EmitterAABB");
		LoadMinMax(area.aabb.area);
		JSON_PARENT();
		JSON_OBJECT("EmitterSphere");
		JSON_LOAD_BY_NAME("radius", area.sphere.radius);
		JSON_OBJECT("distanceFactor");
		LoadMinMax(area.sphere.distanceFactor);
		JSON_PARENT();
		JSON_PARENT();

		JSON_OBJECT("EmitterCapsule");
		JSON_OBJECT("EmitterSegment");
		JSON_LOAD_BY_NAME("start", area.capsule.segment.origin);
		JSON_LOAD_BY_NAME("end", area.capsule.segment.diff);
		JSON_PARENT();
		JSON_LOAD_BY_NAME("radius", area.capsule.radius);
		JSON_OBJECT("distanceFactor");
		LoadMinMax(area.capsule.distanceFactor);
		JSON_PARENT();
		JSON_PARENT();

		JSON_ROOT();
	}

	void LoadScale(GPUParticleShaderStructs::ScaleAnimation& scale) {
		JSON_OBJECT("ScaleAnimation");
		LoadStartEnd(scale.range);
		JSON_LOAD_BY_NAME("isStaticSize", scale.isStaticSize);
		JSON_LOAD_BY_NAME("isUniformScale", scale.isUniformScale);
		JSON_ROOT();
	}

	void LoadRotate(GPUParticleShaderStructs::RotateAnimation& rotate) {
		JSON_OBJECT("RotateAnimation");
		JSON_OBJECT("rotateSpeed");
		LoadMinMax(rotate.rotateSpeed);
		JSON_PARENT();
		JSON_OBJECT("initializeAngle");
		LoadMinMax(rotate.initializeAngle);
		JSON_ROOT();
	}

	void LoadVelocity(GPUParticleShaderStructs::Velocity3D& velocity) {
		JSON_OBJECT("Velocity3D");
		LoadMinMax(velocity.range);
		JSON_ROOT();

	}

	void LoadColor(GPUParticleShaderStructs::EmitterColor& color) {
		JSON_OBJECT("EmitterColor");
		LoadStartEnd(color.range);
		JSON_LOAD_BY_NAME("isStaticColor", color.isStaticColor);
		JSON_ROOT();
	}

	void LoadFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency) {
		JSON_OBJECT("EmitterFrequency");
		JSON_LOAD_BY_NAME("emitterLife", frequency.emitterLife);
		JSON_LOAD_BY_NAME("interval", frequency.interval);
		JSON_LOAD_BY_NAME("isLoop", frequency.isLoop);
		JSON_ROOT();
	}

	void LoadParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan) {
		JSON_OBJECT("ParticleLifeSpan");
		LoadMinMax(particleLifeSpan.range);
		JSON_ROOT();
	}

	void LoadTextureHandle(uint32_t& texture) {
		std::string path{};
		JSON_LOAD_BY_NAME("textureIndex", path);
		if (!path.empty()) {
			texture = TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->Load(path)).GetDescriptorIndex();
		}
	}

	void LoadCreateParticleNum(uint32_t& createParticleNum) {
		JSON_LOAD_BY_NAME("createParticleNum", createParticleNum);
	}

	void LoadCollisionInfo(GPUParticleShaderStructs::ParticleAttributes& particleAttributes) {
		JSON_OBJECT("CollisionInfo");
		JSON_LOAD_BY_NAME("mask", particleAttributes.mask);
		JSON_LOAD_BY_NAME("attribute", particleAttributes.attribute);
		JSON_ROOT();
	}

	void LoadField(GPUParticleShaderStructs::Field& field) {
		JSON_OBJECT("Field");
		JSON_OBJECT("Attraction");
		JSON_LOAD_BY_NAME("attraction", field.attraction.attraction);
		JSON_PARENT();
		JSON_OBJECT("ExternalForce");
		JSON_LOAD_BY_NAME("externalForce", field.externalForce.externalForce);
		JSON_PARENT();
		JSON_LOAD_BY_NAME("type", field.type);
		JSON_ROOT();
	}

	void LoadFieldArea(GPUParticleShaderStructs::EmitterArea& area) {
		JSON_OBJECT("FieldArea");
		JSON_LOAD_BY_NAME("position", area.position);
		JSON_OBJECT("FieldAABB");
		LoadMinMax(area.aabb.area);
		JSON_PARENT();

		JSON_OBJECT("FieldSphere");
		JSON_LOAD_BY_NAME("radius", area.sphere.radius);
		JSON_PARENT();

		JSON_OBJECT("FieldCapsule");
		JSON_OBJECT("FieldSegment");
		JSON_LOAD_BY_NAME("start", area.capsule.segment.origin);
		JSON_LOAD_BY_NAME("end", area.capsule.segment.diff);
		JSON_PARENT();
		JSON_LOAD_BY_NAME("radius", area.capsule.radius);
		JSON_ROOT();
	}

	void LoadFieldFrequency(GPUParticleShaderStructs::FieldFrequency& fieldFrequency) {
		JSON_OBJECT("FieldFrequency");
		JSON_LOAD_BY_NAME("isLoop", fieldFrequency.isLoop);
		JSON_LOAD_BY_NAME("lifeCount", fieldFrequency.lifeCount);
		JSON_ROOT();
	}

	void SaveArea(GPUParticleShaderStructs::EmitterArea& area) {
		JSON_OBJECT("EmitterArea");
		JSON_SAVE_BY_NAME("position", area.position);
		JSON_SAVE_BY_NAME("type", area.type);
		JSON_OBJECT("EmitterAABB");
		SaveMinMax(area.aabb.area);
		JSON_PARENT();
		JSON_OBJECT("EmitterSphere");
		JSON_SAVE_BY_NAME("radius", area.sphere.radius);
		JSON_OBJECT("distanceFactor");
		SaveMinMax(area.sphere.distanceFactor);
		JSON_PARENT();
		JSON_PARENT();


		JSON_OBJECT("EmitterCapsule");
		JSON_OBJECT("EmitterSegment");
		JSON_SAVE_BY_NAME("start", area.capsule.segment.origin);
		JSON_SAVE_BY_NAME("end", area.capsule.segment.diff);
		JSON_PARENT();
		JSON_SAVE_BY_NAME("radius", area.capsule.radius);
		JSON_OBJECT("distanceFactor");
		SaveMinMax(area.capsule.distanceFactor);
		JSON_PARENT();
		JSON_PARENT();

		JSON_ROOT();
	}

	void SaveScale(GPUParticleShaderStructs::ScaleAnimation& scale) {
		JSON_OBJECT("ScaleAnimation");
		SaveStartEnd(scale.range);
		JSON_SAVE_BY_NAME("isStaticSize", scale.isStaticSize);
		JSON_SAVE_BY_NAME("isUniformScale", scale.isUniformScale);
		JSON_ROOT();
	}

	void SaveRotate(GPUParticleShaderStructs::RotateAnimation& rotate) {
		JSON_OBJECT("RotateAnimation");
		JSON_OBJECT("rotateSpeed");
		SaveMinMax(rotate.rotateSpeed);
		JSON_PARENT();
		JSON_OBJECT("initializeAngle");
		SaveMinMax(rotate.initializeAngle);
		JSON_ROOT();
	}

	void SaveVelocity(GPUParticleShaderStructs::Velocity3D& velocity) {
		JSON_OBJECT("Velocity3D");
		SaveMinMax(velocity.range);
		JSON_ROOT();

	}

	void SaveColor(GPUParticleShaderStructs::EmitterColor& color) {
		JSON_OBJECT("EmitterColor");
		SaveStartEnd(color.range);
		JSON_SAVE_BY_NAME("isStaticColor", color.isStaticColor);
		JSON_ROOT();
	}

	void SaveFrequency(GPUParticleShaderStructs::EmitterFrequency& frequency) {
		JSON_OBJECT("EmitterFrequency");
		JSON_SAVE_BY_NAME("emitterLife", frequency.emitterLife);
		JSON_SAVE_BY_NAME("interval", frequency.interval);
		JSON_SAVE_BY_NAME("isLoop", frequency.isLoop);
		JSON_ROOT();
	}

	void SaveParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan) {
		JSON_OBJECT("ParticleLifeSpan");
		SaveMinMax(particleLifeSpan.range);
		JSON_ROOT();
	}

	void SaveTextureHandle(uint32_t& texture) {
		JSON_SAVE_BY_NAME("textureIndex", TextureManager::GetInstance()->GetTexture(TextureManager::GetInstance()->GetTextureLocation(texture)).GetPath().string());

	}

	void SaveCreateParticleNum(uint32_t& createParticleNum) {
		JSON_SAVE_BY_NAME("createParticleNum", createParticleNum);

	}

	void SaveCollisionInfo(GPUParticleShaderStructs::ParticleAttributes& particleAttributes) {
		JSON_OBJECT("CollisionInfo");
		JSON_SAVE_BY_NAME("mask", particleAttributes.mask);
		JSON_SAVE_BY_NAME("attribute", particleAttributes.attribute);
		JSON_ROOT();
	}


	void LoadParent(GPUParticleShaderStructs::EmitterParent& parent) {
		JSON_OBJECT("Parent");
		JSON_LOAD_BY_NAME("isParent", parent.isParent);
		JSON_ROOT();
	}

	void SaveParent(GPUParticleShaderStructs::EmitterParent& parent) {
		JSON_OBJECT("Parent");
		JSON_SAVE_BY_NAME("isParent", parent.isParent);
		JSON_ROOT();
	}

	void SaveField(GPUParticleShaderStructs::Field& field) {
		JSON_OBJECT("Field");
		JSON_OBJECT("Attraction");
		JSON_SAVE_BY_NAME("attraction", field.attraction.attraction);
		JSON_PARENT();
		JSON_OBJECT("ExternalForce");
		JSON_SAVE_BY_NAME("externalForce", field.externalForce.externalForce);
		JSON_PARENT();
		JSON_SAVE_BY_NAME("type", field.type);
		JSON_ROOT();
	}

	void SaveFieldArea(GPUParticleShaderStructs::EmitterArea& area) {
		JSON_OBJECT("FieldArea");
		JSON_SAVE_BY_NAME("position", area.position);
		JSON_OBJECT("FieldAABB");
		LoadMinMax(area.aabb.area);
		JSON_PARENT();

		JSON_OBJECT("FieldSphere");
		JSON_SAVE_BY_NAME("radius", area.sphere.radius);
		JSON_PARENT();

		JSON_OBJECT("FieldCapsule");
		JSON_OBJECT("FieldSegment");
		JSON_SAVE_BY_NAME("start", area.capsule.segment.origin);
		JSON_SAVE_BY_NAME("end", area.capsule.segment.diff);
		JSON_PARENT();
		JSON_SAVE_BY_NAME("radius", area.capsule.radius);
		JSON_ROOT();
	}

	void SaveFieldFrequency(GPUParticleShaderStructs::FieldFrequency& fieldFrequency) {
		JSON_OBJECT("FieldFrequency");
		JSON_SAVE_BY_NAME("isLoop", fieldFrequency.isLoop);
		JSON_SAVE_BY_NAME("lifeCount", fieldFrequency.lifeCount);
		JSON_ROOT();
	}
	std::map<std::string, std::tuple<bool*, EmitterForCPU*, Matrix4x4>>debugEmitters_;
	std::map<std::string, std::tuple<bool*, MeshEmitterDesc*>>debugMeshEmitterDesc_;
	std::map<std::string, std::tuple<bool*, VertexEmitterDesc*>>debugVertexEmitterDesc_;
	std::map<std::string, std::tuple<bool*, TransformEmitter*>>debugTransformEmitter_;
	std::map<std::string, std::tuple<bool*, FieldForCPU*>>debugFields_;
}

void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::EmitterForCPU*, Matrix4x4>e) {
#ifdef _DEBUG
	ImGui::Begin(("Emitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto emitter = std::get<1>(e);
	if (ImGui::Button("Delete")) {
		*std::get<0>(e) = false;
	}
	ImGui::Text("EmitterCount : %d", emitter->emitterCount);

	DrawParent(emitter->parent.isParent);

	DrawArea(emitter->emitterArea);

	DrawScale(emitter->scale);

	DrawRotate(emitter->rotate);

	DrawVelocity(emitter->velocity);

	DrawColor(emitter->color);

	DrawFrequency(emitter->frequency);

	DrawParticleLife(emitter->particleLifeSpan);

	DrawTextureHandle(emitter->textureIndex);

	DrawCreateParticleNum(emitter->createParticleNum);

	DrawCollisionInfo(emitter->collisionInfo);

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

	DrawScale(desc->emitter.scale);

	DrawRotate(desc->emitter.rotate);

	DrawVelocity(desc->emitter.velocity);

	DrawColor(desc->emitter.color);

	DrawParticleLife(desc->emitter.particleLifeSpan);

	DrawTextureHandle(desc->emitter.textureIndex);

	DrawCreateParticleNum(desc->numCreate);

	DrawCollisionInfo(desc->emitter.collisionInfo);


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
	DrawScale(desc->emitter.scale);

	DrawRotate(desc->emitter.rotate);

	DrawVelocity(desc->emitter.velocity);

	DrawColor(desc->emitter.color);

	DrawParticleLife(desc->emitter.particleLifeSpan);

	DrawTextureHandle(desc->emitter.textureIndex);

	DrawCollisionInfo(desc->emitter.collisionInfo);

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *desc);
	}

	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}
void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, TransformEmitter*> e) {

#ifdef _DEBUG
	ImGui::Begin(("TransfromEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto emitter = std::get<1>(e);
	if (ImGui::Button("Delete")) {
		*std::get<0>(e) = false;
	}

	DrawArea(emitter->emitterArea);

	DrawScale(emitter->scale);

	DrawRotate(emitter->rotate);

	DrawVelocity(emitter->velocity);

	DrawColor(emitter->color);

	DrawParticleLife(emitter->particleLifeSpan);

	DrawTextureHandle(emitter->textureIndex);

	DrawCollisionInfo(emitter->collisionInfo);

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *emitter);
	}
	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}
void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::FieldForCPU*> d) {
#ifdef _DEBUG
	ImGui::Begin(("Field:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto emitter = std::get<1>(d);
	if (ImGui::Button("Delete")) {
		*std::get<0>(d) = false;
	}

	DrawArea(emitter->fieldArea);

	DrawField(emitter->field);

	DrawFieldFrequency(emitter->frequency);

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *emitter);
	}
	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter, const Matrix4x4& parent) {
	if (debugEmitters_.find(name) == debugEmitters_.end()) {
		bool* falseFlag = new bool(false);
		debugEmitters_[name] = std::make_tuple(falseFlag, &emitter, parent);
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

void GPUParticleShaderStructs::Debug(const std::string name, TransformEmitter& emitter) {
	if (debugTransformEmitter_.find(name) == debugTransformEmitter_.end()) {
		bool* falseFlag = new bool(false);
		debugTransformEmitter_[name] = std::make_tuple(falseFlag, &emitter);
	}
}

void GPUParticleShaderStructs::Debug(const std::string name, FieldForCPU& desc) {
	if (debugFields_.find(name) == debugFields_.end()) {
		bool* falseFlag = new bool(false);
		debugFields_[name] = std::make_tuple(falseFlag, &desc);
	}
}


void GPUParticleShaderStructs::DebugDraw(const EmitterForCPU& emitter) {
	static const  Vector4 emitterColor = { 0.5f,0.5f,1.0f,1.0f };
	switch (emitter.emitterArea.type) {
	case GPUParticleShaderStructs::Type::kAABB:
	{
		AABB aabb{};
		aabb.min_ = emitter.emitterArea.aabb.area.min;
		aabb.max_ = emitter.emitterArea.aabb.area.max;
		aabb.center_ = emitter.emitterArea.position;
		DrawLine(aabb, emitterColor);
	}
	break;
	case GPUParticleShaderStructs::Type::kSphere:
	{
		Sphere sphere{};
		sphere.center = emitter.emitterArea.position;
		sphere.radius = emitter.emitterArea.sphere.radius;
		DrawLine(sphere, emitterColor);
	}
	break;
	case GPUParticleShaderStructs::Type::kCapsule:
	{
		Capsule capsule{};
		capsule.segment.start = emitter.emitterArea.capsule.segment.origin + emitter.emitterArea.position;
		capsule.segment.end = emitter.emitterArea.capsule.segment.diff + emitter.emitterArea.position;
		capsule.radius = emitter.emitterArea.capsule.radius;
		DrawLine(capsule, emitterColor);
	}
	break;
	default:
		break;
	}
}

void GPUParticleShaderStructs::DebugDraw(const FieldForCPU& emitter) {
	static const  Vector4 emitterColor = { 1.0f,0.5f,0.5f,1.0f };
	switch (emitter.fieldArea.type) {
	case GPUParticleShaderStructs::Type::kAABB:
	{
		AABB aabb{};
		aabb.min_ = emitter.fieldArea.aabb.area.min;
		aabb.max_ = emitter.fieldArea.aabb.area.max;
		aabb.center_ = emitter.fieldArea.position;
		DrawLine(aabb, emitterColor);
	}
	break;
	case GPUParticleShaderStructs::Type::kSphere:
	{
		Sphere sphere{};
		sphere.center = emitter.fieldArea.position;
		sphere.radius = emitter.fieldArea.sphere.radius;
		DrawLine(sphere, emitterColor);
	}
	break;
	case GPUParticleShaderStructs::Type::kCapsule:
	{
		Capsule capsule{};
		capsule.segment.start = emitter.fieldArea.capsule.segment.origin + emitter.fieldArea.position;
		capsule.segment.end = emitter.fieldArea.capsule.segment.diff + emitter.fieldArea.position;
		capsule.radius = emitter.fieldArea.capsule.radius;
		DrawLine(capsule, emitterColor);
	}
	break;
	default:
		break;
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

	if (ImGui::BeginMenu("TransformEmitter")) {
		for (auto& emitter : debugTransformEmitter_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Field")) {
		for (auto& emitter : debugFields_) {
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
	for (auto it = debugTransformEmitter_.begin(); it != debugTransformEmitter_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugTransformEmitter_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
	for (auto it = debugFields_.begin(); it != debugFields_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugFields_.erase(it);
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
	LoadArea(emitter.emitterArea);
	LoadColor(emitter.color);
	LoadFrequency(emitter.frequency);
	LoadParticleLife(emitter.particleLifeSpan);
	LoadRotate(emitter.rotate);
	LoadScale(emitter.scale);
	LoadVelocity(emitter.velocity);
	LoadCollisionInfo(emitter.collisionInfo);
	LoadCreateParticleNum(emitter.createParticleNum);
	LoadTextureHandle(emitter.textureIndex);
	LoadParent(emitter.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/Emitter/" + name + ".json");
	SaveArea(emitter.emitterArea);
	SaveColor(emitter.color);
	SaveFrequency(emitter.frequency);
	SaveParticleLife(emitter.particleLifeSpan);
	SaveRotate(emitter.rotate);
	SaveScale(emitter.scale);
	SaveVelocity(emitter.velocity);
	SaveCollisionInfo(emitter.collisionInfo);
	SaveCreateParticleNum(emitter.createParticleNum);
	SaveTextureHandle(emitter.textureIndex);
	SaveParent(emitter.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");
	LoadScale(desc.emitter.scale);
	LoadParticleLife(desc.emitter.particleLifeSpan);
	LoadColor(desc.emitter.color);
	LoadRotate(desc.emitter.rotate);
	LoadVelocity(desc.emitter.velocity);
	LoadCreateParticleNum(desc.numCreate);
	LoadTextureHandle(desc.emitter.textureIndex);
	LoadCollisionInfo(desc.emitter.collisionInfo);
	JSON_CLOSE();
}



void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");
	SaveScale(desc.emitter.scale);
	SaveParticleLife(desc.emitter.particleLifeSpan);
	SaveColor(desc.emitter.color);
	SaveRotate(desc.emitter.rotate);
	SaveVelocity(desc.emitter.velocity);
	SaveCreateParticleNum(desc.numCreate);
	SaveTextureHandle(desc.emitter.textureIndex);
	SaveCollisionInfo(desc.emitter.collisionInfo);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");
	LoadScale(desc.emitter.scale);
	LoadParticleLife(desc.emitter.particleLifeSpan);
	LoadColor(desc.emitter.color);
	LoadRotate(desc.emitter.rotate);
	LoadVelocity(desc.emitter.velocity);
	LoadTextureHandle(desc.emitter.textureIndex);
	LoadCollisionInfo(desc.emitter.collisionInfo);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, TransformEmitter& emitter) {
	JSON_OPEN("Resources/GPUParticle/TransformEmitter/" + name + ".json");
	SaveArea(emitter.emitterArea);
	SaveScale(emitter.scale);
	SaveRotate(emitter.rotate);
	SaveVelocity(emitter.velocity);
	SaveParticleLife(emitter.particleLifeSpan);
	SaveColor(emitter.color);
	SaveCollisionInfo(emitter.collisionInfo);
	SaveTextureHandle(emitter.textureIndex);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, TransformEmitter& emitter) {
	JSON_OPEN("Resources/GPUParticle/TransformEmitter/" + name + ".json");
	LoadArea(emitter.emitterArea);
	LoadScale(emitter.scale);
	LoadRotate(emitter.rotate);
	LoadVelocity(emitter.velocity);
	LoadParticleLife(emitter.particleLifeSpan);
	LoadColor(emitter.color);
	LoadCollisionInfo(emitter.collisionInfo);
	LoadTextureHandle(emitter.textureIndex);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");
	SaveScale(desc.emitter.scale);
	SaveParticleLife(desc.emitter.particleLifeSpan);
	SaveColor(desc.emitter.color);
	SaveRotate(desc.emitter.rotate);
	SaveVelocity(desc.emitter.velocity);
	SaveTextureHandle(desc.emitter.textureIndex);
	SaveCollisionInfo(desc.emitter.collisionInfo);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, FieldForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/Field/" + name + ".json");
	SaveFieldArea(desc.fieldArea);
	SaveField(desc.field);
	SaveFieldFrequency(desc.frequency);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, FieldForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/Field/" + name + ".json");
	LoadFieldArea(desc.fieldArea);
	LoadField(desc.field);
	LoadFieldFrequency(desc.frequency);
	JSON_CLOSE();
}
