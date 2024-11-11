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
int32_t GPUParticleShaderStructs::VertexEmitterForCPU::staticEmitterCount = 0;
int32_t GPUParticleShaderStructs::MeshEmitterForCPU::staticEmitterCount = 0;
int32_t GPUParticleShaderStructs::TransformModelEmitterForCPU::staticEmitterCount = 0;
int32_t GPUParticleShaderStructs::TransformAreaEmitterForCPU::staticEmitterCount = 0;
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

	void DrawLocalTranslate(GPUParticleShaderStructs::EmitterLocalTransform& localTransform) {
#ifdef _DEBUG
		if (ImGui::TreeNode("LocalTransform")) {
			ImGui::DragFloat3("Translate", &localTransform.translate.x, 0.1f);
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawTranslate(GPUParticleShaderStructs::Translate& translate) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Translate")) {
			ImGui::Checkbox("IsEasing", reinterpret_cast<bool*>(&translate.isEasing));
			if (translate.isEasing) {
				ImGui::DragFloat("Radius", &translate.radius, 0.1f, 0.0f);
				ImGui::DragFloat("Attraction", &translate.attraction, 0.001f, 0.0f);
			}
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

	void DrawAcceleration(GPUParticleShaderStructs::Acceleration3D& acceleration) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Acceleration3D")) {
			DrawMinMax(acceleration.range, 0.1f);
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
			if (!frequency.isOnce) {
				ImGui::DragInt("Interval", reinterpret_cast<int*>(&frequency.interval), 1, 0);
			}
			if (!frequency.isLoop) {
				ImGui::DragInt("EmitterLifeTime", reinterpret_cast<int*>(&frequency.emitterLife), 1, 0);
			}

			ImGui::Checkbox("IsLoop", reinterpret_cast<bool*>(&frequency.isLoop));
			ImGui::Checkbox("IsOnce", reinterpret_cast<bool*>(&frequency.isOnce));
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan) {
#ifdef _DEBUG
		if (ImGui::TreeNode("ParticleLife")) {
			if (!particleLifeSpan.isEmitterLife) {
				DrawMinMax(particleLifeSpan.range);
				particleLifeSpan.isCountDown = false;
			}
			else {
				if (particleLifeSpan.isCountDown) {
					DrawMinMax(particleLifeSpan.range);
				}
				ImGui::Checkbox("isCountDown", reinterpret_cast<bool*>(&particleLifeSpan.isCountDown));
			}
			ImGui::Checkbox("IsEmitterLife", reinterpret_cast<bool*>(&particleLifeSpan.isEmitterLife));
			ImGui::TreePop();
		}
#endif // _DEBUG
	}

	void DrawTrails(GPUParticleShaderStructs::EmitterTrails& emitterTrails) {
#ifdef _DEBUG
		if (ImGui::TreeNode("Trails")) {
			ImGui::Checkbox("IsTrails", reinterpret_cast<bool*>(&emitterTrails.isTrails));
			if (emitterTrails.isTrails) {
				int interval = emitterTrails.interval;
				ImGui::DragInt("Interval", &interval, 1);
				emitterTrails.interval = interval;
				ImGui::DragFloat("Width", &emitterTrails.width, 0.1f);
				ImGui::DragFloat("LifeLimit", &emitterTrails.lifeLimit, 0.1f, 0.0f, 1.0f);
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

					int currentTexture = TextureManager::GetInstance()->GetTextureLocation(emitterTrails.textureIndex);
					// Combo を使用する
					if (ImGui::Combo("Texture", &currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
						emitterTrails.textureIndex = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
					}

					ImGui::TreePop();
				}
			}
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
#ifdef _DEBUG
		bool isParent = static_cast<bool>(parent);

		if (ImGui::Checkbox("IsParent", &isParent)) {
			parent = static_cast<uint32_t>(isParent);
		}
#endif // _DEBUG
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
				ImGui::CheckboxFlags("ParticleField", &particleAttributes.mask, CollisionAttribute::ParticleField);
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
					DrawMinMax(field.externalForce.externalForce, 0.1f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kVelocityRotateForce:
				if (ImGui::TreeNode("VelocityRotateForce")) {
					ImGui::DragFloat3("Direction", &field.velocityRotateForce.direction.x, 0.1f);
					if (field.velocityRotateForce.direction.Length() != 0.0f) {
						field.velocityRotateForce.direction.Normalize();
					}
					ImGui::DragFloat("Speed", &field.velocityRotateForce.rotateSpeed, 0.1f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kPositionRotateForce:
				if (ImGui::TreeNode("PositionRotateForce")) {
					ImGui::DragFloat3("Direction", &field.positionRotateForce.direction.x, 0.1f);
					if (field.positionRotateForce.direction.Length() != 0.0f) {
						field.positionRotateForce.direction.Normalize();
					}
					ImGui::DragFloat("Speed", &field.positionRotateForce.rotateSpeed, 0.1f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kFieldCount:
				break;
			default:
				break;
			}
			std::vector<const char*> typeCStr{ "Attraction","ExternalForce","VelocityRotateForce","PositionRotateForce" };
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

	void LoadLocalTransform(GPUParticleShaderStructs::EmitterLocalTransform& local) {
		JSON_OBJECT("EmitterLocalTransform");
		JSON_LOAD_BY_NAME("translate", local.translate);
		JSON_ROOT();
	}

	void SaveLocalTransform(GPUParticleShaderStructs::EmitterLocalTransform& local) {
		JSON_OBJECT("EmitterLocalTransform");
		JSON_SAVE_BY_NAME("translate", local.translate);
		JSON_ROOT();
	}

	void LoadTranslate(GPUParticleShaderStructs::Translate& translate) {
		JSON_OBJECT("EmitterTranslate");
		JSON_LOAD_BY_NAME("isEasing", translate.isEasing);
		JSON_LOAD_BY_NAME("radius", translate.radius);
		JSON_LOAD_BY_NAME("attraction", translate.attraction);
		JSON_ROOT();
	}

	void SaveTranslate(GPUParticleShaderStructs::Translate& translate) {
		JSON_OBJECT("EmitterTranslate");
		JSON_SAVE_BY_NAME("isEasing", translate.isEasing);
		JSON_SAVE_BY_NAME("radius", translate.radius);
		JSON_SAVE_BY_NAME("attraction", translate.attraction);
		JSON_ROOT();
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
		JSON_LOAD_BY_NAME("isOnce", frequency.isOnce);
		JSON_ROOT();
	}

	void LoadParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan) {
		JSON_OBJECT("ParticleLifeSpan");
		LoadMinMax(particleLifeSpan.range);
		JSON_LOAD_BY_NAME("isEmitterLife", particleLifeSpan.isEmitterLife);
		JSON_LOAD_BY_NAME("isCountDown", particleLifeSpan.isCountDown);
		JSON_ROOT();
	}

	void LoadTrails(GPUParticleShaderStructs::EmitterTrails& emitterTrails) {
		JSON_OBJECT("EmitterTrails");
		JSON_LOAD_BY_NAME("isTrails", emitterTrails.isTrails);
		JSON_LOAD_BY_NAME("textureIndex", emitterTrails.textureIndex);
		JSON_LOAD_BY_NAME("interval", emitterTrails.interval);
		JSON_LOAD_BY_NAME("width", emitterTrails.width);
		JSON_LOAD_BY_NAME("lifeLimit", emitterTrails.lifeLimit);
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
		LoadMinMax(field.externalForce.externalForce);
		JSON_PARENT();
		JSON_OBJECT("VelocityRotateForce");
		JSON_LOAD_BY_NAME("direction", field.velocityRotateForce.direction);
		JSON_LOAD_BY_NAME("rotateSpeed", field.velocityRotateForce.rotateSpeed);
		JSON_PARENT();
		JSON_OBJECT("PositionRotateForce");
		JSON_LOAD_BY_NAME("direction", field.positionRotateForce.direction);
		JSON_LOAD_BY_NAME("rotateSpeed", field.positionRotateForce.rotateSpeed);
		JSON_PARENT();
		JSON_LOAD_BY_NAME("type", field.type);
		JSON_ROOT();
	}

	void LoadFieldArea(GPUParticleShaderStructs::EmitterArea& area) {
		JSON_OBJECT("FieldArea");
		JSON_LOAD_BY_NAME("type", area.type);
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

	void LoadAcceleration(GPUParticleShaderStructs::Acceleration3D& acceleration) {
		JSON_OBJECT("Acceleration3D");
		LoadMinMax(acceleration.range);
		JSON_ROOT();
	}

	void SaveAcceleration(GPUParticleShaderStructs::Acceleration3D& acceleration) {
		JSON_OBJECT("Acceleration3D");
		SaveMinMax(acceleration.range);
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
		JSON_SAVE_BY_NAME("isOnce", frequency.isOnce);
		JSON_ROOT();
	}

	void SaveParticleLife(GPUParticleShaderStructs::ParticleLifeSpan& particleLifeSpan) {
		JSON_OBJECT("ParticleLifeSpan");
		SaveMinMax(particleLifeSpan.range);
		JSON_SAVE_BY_NAME("isEmitterLife", particleLifeSpan.isEmitterLife);
		JSON_SAVE_BY_NAME("isCountDown", particleLifeSpan.isCountDown);
		JSON_ROOT();
	}

	void SaveTrails(GPUParticleShaderStructs::EmitterTrails& emitterTrails) {
		JSON_OBJECT("EmitterTrails");
		JSON_SAVE_BY_NAME("isTrails", emitterTrails.isTrails);
		JSON_SAVE_BY_NAME("textureIndex", emitterTrails.textureIndex);
		JSON_SAVE_BY_NAME("interval", emitterTrails.interval);
		JSON_SAVE_BY_NAME("width", emitterTrails.width);
		JSON_SAVE_BY_NAME("lifeLimit", emitterTrails.lifeLimit);
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
		JSON_SAVE_BY_NAME("attribute", particleAttributes.attribute);
		JSON_SAVE_BY_NAME("mask", particleAttributes.mask);
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
		SaveMinMax(field.externalForce.externalForce);
		JSON_PARENT();
		JSON_OBJECT("VelocityRotateForce");
		JSON_SAVE_BY_NAME("direction", field.velocityRotateForce.direction);
		JSON_SAVE_BY_NAME("rotateSpeed", field.velocityRotateForce.rotateSpeed);
		JSON_PARENT();
		JSON_OBJECT("PositionRotateForce");
		JSON_SAVE_BY_NAME("direction", field.positionRotateForce.direction);
		JSON_SAVE_BY_NAME("rotateSpeed", field.positionRotateForce.rotateSpeed);
		JSON_PARENT();
		JSON_SAVE_BY_NAME("type", field.type);
		JSON_ROOT();
	}

	void SaveFieldArea(GPUParticleShaderStructs::EmitterArea& area) {
		JSON_OBJECT("FieldArea");
		JSON_SAVE_BY_NAME("type", area.type);
		JSON_SAVE_BY_NAME("position", area.position);
		JSON_OBJECT("FieldAABB");
		SaveMinMax(area.aabb.area);
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
	void NonSharedCopy(GPUParticleShaderStructs::EmitterForCPU& dst, const GPUParticleShaderStructs::EmitterForCPU& src) {
		dst.emitterArea = src.emitterArea;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.textureIndex = src.textureIndex;
		dst.createParticleNum = src.createParticleNum;
		dst.isAlive = src.isAlive;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;

	}
	void NonSharedCopy(GPUParticleShaderStructs::VertexEmitterForCPU& dst, const GPUParticleShaderStructs::VertexEmitterForCPU& src) {
		dst.translate = src.translate;
		dst.localTransform = src.localTransform;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.model = src.model;
		dst.textureIndex = src.textureIndex;
		dst.isAlive = src.isAlive;
	}
	void NonSharedCopy(GPUParticleShaderStructs::MeshEmitterForCPU& dst, const GPUParticleShaderStructs::MeshEmitterForCPU& src) {
		dst.translate = src.translate;
		dst.localTransform = src.localTransform;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.model = src.model;
		dst.textureIndex = src.textureIndex;
		dst.numCreate = src.numCreate;
		dst.isAlive = src.isAlive;
	}
	void NonSharedCopy(GPUParticleShaderStructs::TransformAreaEmitterForCPU& dst, const GPUParticleShaderStructs::TransformAreaEmitterForCPU& src) {
		dst.emitterArea = src.emitterArea;
		dst.translate = src.translate;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.model = src.model;
		dst.modelWorldMatrix = src.modelWorldMatrix;
		dst.textureIndex = src.textureIndex;
		dst.isAlive = src.isAlive;
	}
	void NonSharedCopy(GPUParticleShaderStructs::TransformModelEmitterForCPU& dst, const GPUParticleShaderStructs::TransformModelEmitterForCPU& src) {
		dst.translate = src.translate;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.startModel = src.startModel;
		dst.startModelWorldMatrix = src.startModelWorldMatrix;
		dst.endModel = src.endModel;
		dst.endModelWorldMatrix = src.endModelWorldMatrix;
		dst.textureIndex = src.textureIndex;
		dst.isAlive = src.isAlive;
	}
	void NonSharedCopy(GPUParticleShaderStructs::FieldForCPU& dst, const GPUParticleShaderStructs::FieldForCPU& src) {
		dst.field = src.field;
		dst.fieldArea = src.fieldArea;
		dst.frequency = src.frequency;
		dst.collisionInfo = src.collisionInfo;
		dst.isAlive = src.isAlive;
	}
	void Copy(GPUParticleShaderStructs::EmitterForGPU& dst, const GPUParticleShaderStructs::EmitterForCPU& src, const Matrix4x4& parent) {
		dst.emitterArea = src.emitterArea;
		dst.scale = src.scale;
		dst.rotate.initializeAngle.min = DegToRad(src.rotate.initializeAngle.min);
		dst.rotate.initializeAngle.max = DegToRad(src.rotate.initializeAngle.max);
		dst.rotate.rotateSpeed.min = DegToRad(src.rotate.rotateSpeed.min);
		dst.rotate.rotateSpeed.max = DegToRad(src.rotate.rotateSpeed.max);
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time.particleTime = src.frequency.interval;
		dst.time.emitterTime = 0;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.textureIndex = src.textureIndex;
		dst.createParticleNum = src.createParticleNum;
		dst.isAlive = src.isAlive;
		dst.emitterCount = src.emitterCount;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.parent.emitterType = GPUParticleShaderStructs::EmitterType::kEmitter;
		dst.parent.worldMatrix = parent;
	}
	void Copy(GPUParticleShaderStructs::VertexEmitterForGPU& dst, const GPUParticleShaderStructs::VertexEmitterForCPU& src, const Matrix4x4& parent) {
		dst.localTransform = src.localTransform;
		dst.translate = src.translate;
		dst.scale = src.scale;
		dst.rotate.initializeAngle.min = DegToRad(src.rotate.initializeAngle.min);
		dst.rotate.initializeAngle.max = DegToRad(src.rotate.initializeAngle.max);
		dst.rotate.rotateSpeed.min = DegToRad(src.rotate.rotateSpeed.min);
		dst.rotate.rotateSpeed.max = DegToRad(src.rotate.rotateSpeed.max);
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.parent.emitterType = GPUParticleShaderStructs::EmitterType::kVertexEmitter;
		dst.parent.worldMatrix = parent;
		dst.model = src.model;
		dst.textureIndex = src.textureIndex;
		dst.isAlive = src.isAlive;
		dst.emitterCount = src.emitterCount;
	}
	void Copy(GPUParticleShaderStructs::MeshEmitterForGPU& dst, const GPUParticleShaderStructs::MeshEmitterForCPU& src, const Matrix4x4& parent) {
		dst.localTransform = src.localTransform;
		dst.translate = src.translate;
		dst.scale = src.scale;
		dst.rotate.initializeAngle.min = DegToRad(src.rotate.initializeAngle.min);
		dst.rotate.initializeAngle.max = DegToRad(src.rotate.initializeAngle.max);
		dst.rotate.rotateSpeed.min = DegToRad(src.rotate.rotateSpeed.min);
		dst.rotate.rotateSpeed.max = DegToRad(src.rotate.rotateSpeed.max);
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.parent.emitterType = GPUParticleShaderStructs::EmitterType::kMeshEmitter;
		dst.parent.worldMatrix = parent;
		dst.model = src.model;
		dst.textureIndex = src.textureIndex;
		dst.numCreate = src.numCreate;
		dst.isAlive = src.isAlive;
		dst.emitterCount = src.emitterCount;
	}
	void Copy(GPUParticleShaderStructs::TransformAreaEmitterForGPU& dst, const GPUParticleShaderStructs::TransformAreaEmitterForCPU& src, const Matrix4x4& parent) {
		dst.emitterArea = src.emitterArea;
		dst.translate = src.translate;
		dst.scale = src.scale;
		dst.rotate.initializeAngle.min = DegToRad(src.rotate.initializeAngle.min);
		dst.rotate.initializeAngle.max = DegToRad(src.rotate.initializeAngle.max);
		dst.rotate.rotateSpeed.min = DegToRad(src.rotate.rotateSpeed.min);
		dst.rotate.rotateSpeed.max = DegToRad(src.rotate.rotateSpeed.max);
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.parent.emitterType = GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter;
		dst.parent.worldMatrix = parent;
		dst.model = src.model;
		dst.modelWorldMatrix = src.modelWorldMatrix;
		dst.textureIndex = src.textureIndex;
		dst.isAlive = src.isAlive;
		dst.emitterCount = src.emitterCount;
	}
	void Copy(GPUParticleShaderStructs::TransformModelEmitterForGPU& dst, const GPUParticleShaderStructs::TransformModelEmitterForCPU& src, const Matrix4x4& parent) {
		dst.translate = src.translate;
		dst.scale = src.scale;
		dst.rotate.initializeAngle.min = DegToRad(src.rotate.initializeAngle.min);
		dst.rotate.initializeAngle.max = DegToRad(src.rotate.initializeAngle.max);
		dst.rotate.rotateSpeed.min = DegToRad(src.rotate.rotateSpeed.min);
		dst.rotate.rotateSpeed.max = DegToRad(src.rotate.rotateSpeed.max);
		dst.velocity = src.velocity;
		dst.acceleration = src.acceleration;
		dst.color = src.color;
		dst.frequency = src.frequency;
		dst.time = src.time;
		dst.particleLifeSpan = src.particleLifeSpan;
		dst.emitterTrails = src.emitterTrails;
		dst.collisionInfo = src.collisionInfo;
		dst.parent = src.parent;
		dst.parent.emitterType = GPUParticleShaderStructs::EmitterType::kTransformModelEmitter;
		dst.parent.worldMatrix = parent;
		dst.startModel = src.startModel;
		dst.startModelWorldMatrix = src.startModelWorldMatrix;
		dst.endModel = src.endModel;
		dst.endModelWorldMatrix = src.endModelWorldMatrix;
		dst.textureIndex = src.textureIndex;
		dst.isAlive = src.isAlive;
		dst.emitterCount = src.emitterCount;
	}
	void Copy(GPUParticleShaderStructs::FieldForGPU& dst, const GPUParticleShaderStructs::FieldForCPU& src) {
		dst.field = src.field;
		dst.field.velocityRotateForce.rotateSpeed = DegToRad(dst.field.velocityRotateForce.rotateSpeed);
		dst.field.positionRotateForce.rotateSpeed = DegToRad(dst.field.positionRotateForce.rotateSpeed);
		dst.fieldArea = src.fieldArea;
		dst.frequency = src.frequency;
		dst.collisionInfo = src.collisionInfo;
		dst.isAlive = src.isAlive;
		dst.fieldCount = src.fieldCount;
	}
	std::map<std::string, std::tuple<bool*, EmitterForCPU*, Matrix4x4>>debugEmitters_;
	std::map<std::string, std::tuple<bool*, MeshEmitterForCPU*>>debugMeshEmitter_;
	std::map<std::string, std::tuple<bool*, VertexEmitterForCPU*>>debugVertexEmitter_;
	std::map<std::string, std::tuple<bool*, TransformModelEmitterForCPU*>>debugTransformModelEmitter_;
	std::map<std::string, std::tuple<bool*, TransformAreaEmitterForCPU*>>debugTransformAreaEmitter_;
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

	DrawAcceleration(emitter->acceleration);

	DrawColor(emitter->color);

	DrawFrequency(emitter->frequency);

	DrawParticleLife(emitter->particleLifeSpan);

	DrawTrails(emitter->emitterTrails);

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

void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::MeshEmitterForCPU*> d) {
#ifdef _DEBUG
	ImGui::Begin(("MeshEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto& desc = std::get<1>(d);
	if (ImGui::Button("Delete")) {
		*std::get<0>(d) = false;

	}
	ImGui::Text("EmitterCount : %d", desc->emitterCount);

	DrawParent(desc->parent.isParent);

	DrawLocalTranslate(desc->localTransform);

	DrawTranslate(desc->translate);

	DrawScale(desc->scale);

	DrawRotate(desc->rotate);

	DrawVelocity(desc->velocity);

	DrawAcceleration(desc->acceleration);

	DrawColor(desc->color);

	DrawFrequency(desc->frequency);

	DrawParticleLife(desc->particleLifeSpan);

	DrawTrails(desc->emitterTrails);

	DrawTextureHandle(desc->textureIndex);

	DrawCreateParticleNum(desc->numCreate);

	DrawCollisionInfo(desc->collisionInfo);


	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *desc);
	}

	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}

void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, GPUParticleShaderStructs::VertexEmitterForCPU*> d) {
#ifdef _DEBUG
	ImGui::Begin(("VertexEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto& desc = std::get<1>(d);
	if (ImGui::Button("Delete")) {
		*std::get<0>(d) = false;
	}
	ImGui::Text("EmitterCount : %d", desc->emitterCount);

	DrawParent(desc->parent.isParent);

	DrawLocalTranslate(desc->localTransform);

	DrawTranslate(desc->translate);

	DrawScale(desc->scale);

	DrawRotate(desc->rotate);

	DrawVelocity(desc->velocity);

	DrawAcceleration(desc->acceleration);

	DrawColor(desc->color);

	DrawFrequency(desc->frequency);

	DrawParticleLife(desc->particleLifeSpan);

	DrawTrails(desc->emitterTrails);

	DrawTextureHandle(desc->textureIndex);

	DrawCollisionInfo(desc->collisionInfo);

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *desc);
	}

	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}
void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, TransformModelEmitterForCPU*> e) {

#ifdef _DEBUG
	ImGui::Begin(("TransfromModelEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto emitter = std::get<1>(e);
	if (ImGui::Button("Delete")) {
		*std::get<0>(e) = false;
	}
	ImGui::Text("EmitterCount : %d", emitter->emitterCount);

	DrawParent(emitter->parent.isParent);

	DrawTranslate(emitter->translate);

	DrawScale(emitter->scale);

	DrawRotate(emitter->rotate);

	DrawVelocity(emitter->velocity);

	DrawAcceleration(emitter->acceleration);

	DrawColor(emitter->color);

	DrawFrequency(emitter->frequency);

	DrawParticleLife(emitter->particleLifeSpan);

	DrawTrails(emitter->emitterTrails);

	DrawTextureHandle(emitter->textureIndex);

	DrawCollisionInfo(emitter->collisionInfo);

	if (ImGui::Button("Save")) {
		GPUParticleShaderStructs::Save(name, *emitter);
	}
	ImGui::PopID();
	ImGui::End();
#endif // _DEBUG
}
void GPUParticleShaderStructs::EmitterEditor(const std::string name, std::tuple<bool*, TransformAreaEmitterForCPU*> e) {

#ifdef _DEBUG
	ImGui::Begin(("TransfromAreaEmitter:" + name).c_str());
	ImGui::PushID(name.c_str());
	auto emitter = std::get<1>(e);
	if (ImGui::Button("Delete")) {
		*std::get<0>(e) = false;
	}
	ImGui::Text("EmitterCount : %d", emitter->emitterCount);

	DrawParent(emitter->parent.isParent);

	DrawArea(emitter->emitterArea);

	DrawTranslate(emitter->translate);

	DrawScale(emitter->scale);

	DrawRotate(emitter->rotate);

	DrawVelocity(emitter->velocity);

	DrawAcceleration(emitter->acceleration);

	DrawColor(emitter->color);

	DrawFrequency(emitter->frequency);

	DrawParticleLife(emitter->particleLifeSpan);

	DrawTrails(emitter->emitterTrails);

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

	DrawCollisionInfo(emitter->collisionInfo);

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

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::MeshEmitterForCPU& emitter) {
	if (debugMeshEmitter_.find(name) == debugMeshEmitter_.end()) {
		bool* falseFlag = new bool(false);
		debugMeshEmitter_[name] = std::make_tuple(falseFlag, &emitter);
	}
}


void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::VertexEmitterForCPU& emitter) {
	if (debugVertexEmitter_.find(name) == debugVertexEmitter_.end()) {
		bool* falseFlag = new bool(false);
		debugVertexEmitter_[name] = std::make_tuple(falseFlag, &emitter);
	}
}

void GPUParticleShaderStructs::Debug(const std::string name, TransformModelEmitterForCPU& emitter) {
	if (debugTransformModelEmitter_.find(name) == debugTransformModelEmitter_.end()) {
		bool* falseFlag = new bool(false);
		debugTransformModelEmitter_[name] = std::make_tuple(falseFlag, &emitter);
	}
}

void GPUParticleShaderStructs::Debug(const std::string name, TransformAreaEmitterForCPU& emitter) {
	if (debugTransformAreaEmitter_.find(name) == debugTransformAreaEmitter_.end()) {
		bool* falseFlag = new bool(false);
		debugTransformAreaEmitter_[name] = std::make_tuple(falseFlag, &emitter);
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
		capsule.segment.start = emitter.emitterArea.capsule.segment.origin ;
		capsule.segment.end = emitter.emitterArea.capsule.segment.diff;
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
		capsule.segment.start = emitter.fieldArea.capsule.segment.origin;
		capsule.segment.end = emitter.fieldArea.capsule.segment.diff;
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
		for (auto& emitter : debugMeshEmitter_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("VertexParticle")) {
		for (auto& emitter : debugVertexEmitter_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("TransformModelEmitter")) {
		for (auto& emitter : debugTransformModelEmitter_) {
			bool* debugFlag = std::get<0>(emitter.second);
			if (ImGui::Button(emitter.first.c_str()) &&
				!(*debugFlag)) {
				*debugFlag = true;
			}
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("TransformAreaEmitter")) {
		for (auto& emitter : debugTransformAreaEmitter_) {
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
	for (auto it = debugMeshEmitter_.begin(); it != debugMeshEmitter_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugMeshEmitter_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
	for (auto it = debugVertexEmitter_.begin(); it != debugVertexEmitter_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugVertexEmitter_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
	for (auto it = debugTransformModelEmitter_.begin(); it != debugTransformModelEmitter_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugTransformModelEmitter_.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			++it;
		}
	}
	for (auto it = debugTransformAreaEmitter_.begin(); it != debugTransformAreaEmitter_.end(); ) {
		if (*(std::get<0>(it->second))) {
			EmitterEditor(it->first, it->second);
			if (!*(std::get<0>(it->second))) {
				it = debugTransformAreaEmitter_.erase(it);
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
	LoadTrails(emitter.emitterTrails);
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
	SaveTrails(emitter.emitterTrails);
	SaveRotate(emitter.rotate);
	SaveScale(emitter.scale);
	SaveVelocity(emitter.velocity);
	SaveCollisionInfo(emitter.collisionInfo);
	SaveCreateParticleNum(emitter.createParticleNum);
	SaveTextureHandle(emitter.textureIndex);
	SaveParent(emitter.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::MeshEmitterForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");
	LoadLocalTransform(desc.localTransform);
	LoadTranslate(desc.translate);
	LoadScale(desc.scale);
	LoadParticleLife(desc.particleLifeSpan);
	LoadTrails(desc.emitterTrails);
	LoadColor(desc.color);
	LoadFrequency(desc.frequency);
	LoadRotate(desc.rotate);
	LoadVelocity(desc.velocity);
	LoadCreateParticleNum(desc.numCreate);
	LoadTextureHandle(desc.textureIndex);
	LoadCollisionInfo(desc.collisionInfo);
	LoadParent(desc.parent);
	JSON_CLOSE();
}



void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::MeshEmitterForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");
	SaveLocalTransform(desc.localTransform);
	SaveTranslate(desc.translate);
	SaveScale(desc.scale);
	SaveParticleLife(desc.particleLifeSpan);
	SaveTrails(desc.emitterTrails);
	SaveColor(desc.color);
	SaveFrequency(desc.frequency);
	SaveRotate(desc.rotate);
	SaveVelocity(desc.velocity);
	SaveCreateParticleNum(desc.numCreate);
	SaveTextureHandle(desc.textureIndex);
	SaveCollisionInfo(desc.collisionInfo);
	SaveParent(desc.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::VertexEmitterForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");
	LoadLocalTransform(desc.localTransform);
	LoadTranslate(desc.translate);
	LoadScale(desc.scale);
	LoadParticleLife(desc.particleLifeSpan);
	LoadTrails(desc.emitterTrails);
	LoadColor(desc.color);
	LoadFrequency(desc.frequency);
	LoadRotate(desc.rotate);
	LoadVelocity(desc.velocity);
	LoadTextureHandle(desc.textureIndex);
	LoadCollisionInfo(desc.collisionInfo);
	LoadParent(desc.parent);
	JSON_CLOSE();
}
void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::VertexEmitterForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");
	SaveLocalTransform(desc.localTransform);
	SaveTranslate(desc.translate);
	SaveScale(desc.scale);
	SaveParticleLife(desc.particleLifeSpan);
	SaveTrails(desc.emitterTrails);
	SaveColor(desc.color);
	SaveFrequency(desc.frequency);
	SaveRotate(desc.rotate);
	SaveVelocity(desc.velocity);
	SaveTextureHandle(desc.textureIndex);
	SaveCollisionInfo(desc.collisionInfo);
	SaveParent(desc.parent);
	JSON_CLOSE();
}


void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::TransformModelEmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/TransformEmitter/" + name + ".json");
	SaveScale(emitter.scale);
	SaveTranslate(emitter.translate);
	SaveRotate(emitter.rotate);
	SaveVelocity(emitter.velocity);
	SaveParticleLife(emitter.particleLifeSpan);
	SaveTrails(emitter.emitterTrails);
	SaveColor(emitter.color);
	SaveFrequency(emitter.frequency);
	SaveCollisionInfo(emitter.collisionInfo);
	SaveTextureHandle(emitter.textureIndex);
	SaveParent(emitter.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::TransformModelEmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/TransformEmitter/" + name + ".json");
	LoadScale(emitter.scale);
	LoadTranslate(emitter.translate);
	LoadRotate(emitter.rotate);
	LoadVelocity(emitter.velocity);
	LoadParticleLife(emitter.particleLifeSpan);
	LoadTrails(emitter.emitterTrails);
	LoadColor(emitter.color);
	LoadFrequency(emitter.frequency);
	LoadCollisionInfo(emitter.collisionInfo);
	LoadTextureHandle(emitter.textureIndex);
	LoadParent(emitter.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::TransformAreaEmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/TransformEmitter/" + name + ".json");
	SaveArea(emitter.emitterArea);
	SaveTranslate(emitter.translate);
	SaveScale(emitter.scale);
	SaveRotate(emitter.rotate);
	SaveVelocity(emitter.velocity);
	SaveParticleLife(emitter.particleLifeSpan);
	SaveTrails(emitter.emitterTrails);
	SaveColor(emitter.color);
	SaveFrequency(emitter.frequency);
	SaveCollisionInfo(emitter.collisionInfo);
	SaveTextureHandle(emitter.textureIndex);
	SaveParent(emitter.parent);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::TransformAreaEmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/TransformEmitter/" + name + ".json");
	LoadArea(emitter.emitterArea);
	LoadTranslate(emitter.translate);
	LoadScale(emitter.scale);
	LoadRotate(emitter.rotate);
	LoadVelocity(emitter.velocity);
	LoadParticleLife(emitter.particleLifeSpan);
	LoadTrails(emitter.emitterTrails);
	LoadColor(emitter.color);
	LoadFrequency(emitter.frequency);
	LoadCollisionInfo(emitter.collisionInfo);
	LoadTextureHandle(emitter.textureIndex);
	LoadParent(emitter.parent);
	JSON_CLOSE();
}


void GPUParticleShaderStructs::Save(const std::string name, FieldForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/Field/" + name + ".json");
	SaveFieldArea(desc.fieldArea);
	SaveField(desc.field);
	SaveFieldFrequency(desc.frequency);
	SaveCollisionInfo(desc.collisionInfo);
	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, FieldForCPU& desc) {
	JSON_OPEN("Resources/GPUParticle/Field/" + name + ".json");
	LoadFieldArea(desc.fieldArea);
	LoadField(desc.field);
	LoadFieldFrequency(desc.frequency);
	LoadCollisionInfo(desc.collisionInfo);
	JSON_CLOSE();
}
