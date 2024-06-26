#include "GPUParticleShaderStructs.h"

#include <list>

#include "Engine/Json/JsonUtils.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"

int32_t GPUParticleShaderStructs::EmitterForCPU::staticEmitterCount = 0;

namespace {

	void DrawMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f) {
		ImGui::DragFloat3("Min", &startEnd.min.x, v_speed, v_min, v_max);
		ImGui::DragFloat3("Max", &startEnd.max.x, v_speed, v_min, v_max);
	}

	void DrawMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f) {
		ImGui::DragFloat4("Min", &startEnd.min.x, v_speed, v_min, v_max);
		ImGui::DragFloat4("Max", &startEnd.max.x, v_speed, v_min, v_max);
	}

	void DrawStartEnd(GPUParticleShaderStructs::Vector3StartEnd& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f) {
		if (ImGui::TreeNode("Start")) {
			DrawMinMax(startEnd.start, v_speed, v_min, v_max);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("End")) {
			DrawMinMax(startEnd.end, v_speed, v_min, v_max);
			ImGui::TreePop();
		}
	}
	void DrawStartEnd(GPUParticleShaderStructs::Vector4StartEnd& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f) {
		if (ImGui::TreeNode("Start")) {
			DrawMinMax(startEnd.start, v_speed, v_min, v_max);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("End")) {
			DrawMinMax(startEnd.end, v_speed, v_min, v_max);
			ImGui::TreePop();
		}
	}
}

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	ImGui::Begin("GPUParticle");
	if (ImGui::BeginMenu(name.c_str())) {
		ImGui::PushID(name.c_str());
		ImGui::Text("EmitterCount : %d", emitter.emitterCount);
		if (ImGui::TreeNode("Area")) {
			switch (emitter.emitterArea.type) {
			case GPUParticleShaderStructs::kAABB:
				if (ImGui::TreeNode("AABB")) {
					DrawMinMax(emitter.emitterArea.aabb.area);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kSphere:
				if (ImGui::TreeNode("Sphere")) {
					ImGui::DragFloat("Radius", &emitter.emitterArea.sphere.radius, 0.1f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kCapsule:
				if (ImGui::TreeNode("Capsule")) {
					ImGui::DragFloat3("Start", &emitter.emitterArea.capsule.segment.origin.x, 0.1f);
					ImGui::DragFloat3("End", &emitter.emitterArea.capsule.segment.diff.x, 0.1f);
					ImGui::DragFloat("Radius", &emitter.emitterArea.capsule.radius, 0.1f);
					ImGui::TreePop();
				}
				break;
			case GPUParticleShaderStructs::kCount:
				break;
			default:
				break;
			}

			ImGui::DragFloat3("Position", &emitter.emitterArea.position.x, 0.1f);
			std::vector<const char*> stateNamesCStr{ "AABB","Sphere","Capsule" };
			int currentState = static_cast<int>(emitter.emitterArea.type);

			// ステートを変更するImGui::Comboの作成
			if (ImGui::Combo("Type", &currentState, stateNamesCStr.data(), int(stateNamesCStr.size()))) {
				emitter.emitterArea.type = static_cast<Type>(currentState);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Scale")) {
			DrawStartEnd(emitter.scale.range, 0.1f, 0.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Rotate")) {
			ImGui::DragFloat("rotate", &emitter.rotate.rotate, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Velocity3D")) {
			DrawMinMax(emitter.velocity.range,0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Color")) {
			DrawStartEnd(emitter.color.range, 0.01f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Frequency")) {
			ImGui::DragInt("Interval", reinterpret_cast<int*>(&emitter.frequency.interval), 1, 0);
			if (!emitter.frequency.isLoop) {
				ImGui::SliderInt("EmitterLifeTime", reinterpret_cast<int*>(&emitter.frequency.emitterLife), 1, 0);
			}
			ImGui::Checkbox("IsLoop", reinterpret_cast<bool*>(&emitter.frequency.isLoop));
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("ParticleLife")) {
			ImGui::DragInt("Max", reinterpret_cast<int*>(&emitter.particleLifeSpan.range.max), 1, 0);
			ImGui::DragInt("Min", reinterpret_cast<int*>(&emitter.particleLifeSpan.range.min), 1, 0);
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

			// Combo を使用する
			if (ImGui::Combo("Texture", reinterpret_cast<int*>(&emitter.textureIndex), stageArray.data(), static_cast<int>(stageArray.size()))) {
				emitter.textureIndex = TextureManager::GetInstance()->GetTexture(emitter.textureIndex).GetDescriptorIndex();
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("CreateParticle")) {
			ImGui::DragInt("Num", reinterpret_cast<int*>(&emitter.createParticleNum));
			ImGui::TreePop();
		}
		ImGui::PopID();
		ImGui::EndMenu();
	}
	ImGui::End();
}
//
//GPUParticleShaderStructs::EmitterForCPU GPUParticleShaderStructs::Load(const std::string name) {}
//
//void GPUParticleShaderStructs::Save(const std::string name, EmitterForCPU& emitter) {
//	JSON_OPEN("Resources/GPUParticleData/" + name + ".json");
//	JSON_OBJECT("Area");
//	JSON_LOAD(emitter)
//
//}
