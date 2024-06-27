#include "GPUParticleShaderStructs.h"

#include <fstream>
#include <list>

#include "Engine/Json/JsonUtils.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"

int32_t GPUParticleShaderStructs::EmitterForCPU::staticEmitterCount = 0;

namespace {

	void SaveMinMax(GPUParticleShaderStructs::UintMinMax& startEnd) {
		JSON_SAVE_BY_NAME("Min", startEnd.min);
		JSON_SAVE_BY_NAME("Max", startEnd.max);
	}

	void LoadMinMax(GPUParticleShaderStructs::UintMinMax& startEnd) {
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

	void DrawMinMax(GPUParticleShaderStructs::UintMinMax& startEnd, float v_speed = 1.0f, int v_min = 0, int v_max = 0) {

		int min = static_cast<int>(startEnd.min);
		int max = static_cast<int>(startEnd.max);

		ImGui::DragInt("Min", &min, v_speed, v_min, max);
		ImGui::DragInt("Max", &max, v_speed, min, v_max);

		startEnd.min = static_cast<uint32_t>(min);
		startEnd.max = static_cast<uint32_t>(max);
	}

	void DrawMinMax(GPUParticleShaderStructs::Vector3MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f) {

		ImGui::DragFloat3("Min", &startEnd.min.x, v_speed, v_min, v_max);
		ImGui::DragFloat3("Max", &startEnd.max.x, v_speed, v_min, v_max);
	}

	void DrawMinMax(GPUParticleShaderStructs::Vector4MinMax& startEnd, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f) {
		ImGui::DragFloat4("Min", &startEnd.min.x, v_speed, v_min, v_max);
		ImGui::DragFloat4("Max", &startEnd.max.x, v_speed, v_min, v_max);
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
	if (ImGui::BeginMenu("Emitter")) {
		if (ImGui::TreeNode(name.c_str())) {
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
				DrawStartEnd(emitter.scale.range, 0.01f, 0.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Rotate")) {
				ImGui::DragFloat("rotate", &emitter.rotate.rotate, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Velocity3D")) {
				DrawMinMax(emitter.velocity.range, 0.1f);
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
				DrawMinMax(emitter.particleLifeSpan.range);
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
			if (ImGui::Button("Save")) {
				GPUParticleShaderStructs::Save(name, emitter);
			}

			ImGui::PopID();
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
}

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	ImGui::Begin("GPUParticle");
	if (ImGui::BeginMenu("MeshParticle")) {
		if (ImGui::TreeNode(name.c_str())) {
			ImGui::PushID(name.c_str());
			if (ImGui::TreeNode("Scale")) {
				DrawStartEnd(desc.emitter.scale.range, 0.01f, 0.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Rotate")) {
				ImGui::DragFloat("rotate", &desc.emitter.rotate.rotate, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Velocity3D")) {
				DrawMinMax(desc.emitter.velocity.range, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Color")) {
				DrawStartEnd(desc.emitter.color.range, 0.01f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("ParticleLife")) {
				DrawMinMax(desc.emitter.particleLifeSpan.range);
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

				int currentTexture = TextureManager::GetInstance()->GetTextureLocation(desc.emitter.textureIndex);
				// Combo を使用する
				if (ImGui::Combo("Texture",&currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
					desc.emitter.textureIndex = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("NumCreate")) {
				int num = desc.numCreate;
				ImGui::DragInt("num", &num, 1.0f, 0,50);
				desc.numCreate = num;
				ImGui::TreePop();
			}
			if (ImGui::Button("Save")) {
				GPUParticleShaderStructs::Save(name, desc);
			}

			ImGui::PopID();
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
}

void GPUParticleShaderStructs::Debug(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	ImGui::Begin("GPUParticle");
	if (ImGui::BeginMenu("VertexParticle")) {
		if (ImGui::TreeNode(name.c_str())) {
			ImGui::PushID(name.c_str());
			if (ImGui::TreeNode("Scale")) {
				DrawStartEnd(desc.emitter.scale.range, 0.01f, 0.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Rotate")) {
				ImGui::DragFloat("rotate", &desc.emitter.rotate.rotate, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Velocity3D")) {
				DrawMinMax(desc.emitter.velocity.range, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Color")) {
				DrawStartEnd(desc.emitter.color.range, 0.01f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("ParticleLife")) {
				DrawMinMax(desc.emitter.particleLifeSpan.range);
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

				int currentTexture = TextureManager::GetInstance()->GetTextureLocation(desc.emitter.textureIndex);
				// Combo を使用する
				if (ImGui::Combo("Texture", &currentTexture, stageArray.data(), static_cast<int>(stageArray.size()))) {
					desc.emitter.textureIndex = TextureManager::GetInstance()->GetTexture(currentTexture).GetDescriptorIndex();
				}

				ImGui::TreePop();
			}

			if (ImGui::Button("Save")) {
				GPUParticleShaderStructs::Save(name, desc);
			}

			ImGui::PopID();
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
}


void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/Emitter/" + name + ".json");

	JSON_OBJECT("EmitterArea");
	JSON_LOAD_BY_NAME("position", emitter.emitterArea.position);
	JSON_LOAD_BY_NAME("type", emitter.emitterArea.type);
	JSON_OBJECT("EmitterAABB");
	LoadMinMax(emitter.emitterArea.aabb.area);
	JSON_PARENT();
	JSON_OBJECT("EmitterSphere");
	JSON_LOAD_BY_NAME("radius", emitter.emitterArea.sphere.radius);
	JSON_PARENT();
	JSON_OBJECT("EmitterCapsule");
	JSON_OBJECT("EmitterSegment");
	JSON_LOAD_BY_NAME("start", emitter.emitterArea.capsule.segment.origin);
	JSON_LOAD_BY_NAME("end", emitter.emitterArea.capsule.segment.diff);
	JSON_PARENT();
	JSON_LOAD_BY_NAME("radius", emitter.emitterArea.capsule.radius);
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
	JSON_LOAD_BY_NAME("rotate", emitter.rotate.rotate);
	JSON_ROOT();

	JSON_OBJECT("ScaleAnimation");
	LoadStartEnd(emitter.scale.range);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	LoadMinMax(emitter.velocity.range);
	JSON_ROOT();

	JSON_LOAD_BY_NAME("createParticleNum", emitter.createParticleNum);
	JSON_LOAD_BY_NAME("textureIndex", emitter.textureIndex);

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::EmitterForCPU& emitter) {
	JSON_OPEN("Resources/GPUParticle/Emitter/" + name + ".json");

	JSON_OBJECT("EmitterArea");
	JSON_SAVE_BY_NAME("position", emitter.emitterArea.position);
	JSON_SAVE_BY_NAME("type", emitter.emitterArea.type);
	JSON_OBJECT("EmitterAABB");
	SaveMinMax(emitter.emitterArea.aabb.area);
	JSON_PARENT();
	JSON_OBJECT("EmitterSphere");
	JSON_SAVE_BY_NAME("radius", emitter.emitterArea.sphere.radius);
	JSON_PARENT();
	JSON_OBJECT("EmitterCapsule");
	JSON_OBJECT("EmitterSegment");
	JSON_SAVE_BY_NAME("start", emitter.emitterArea.capsule.segment.origin);
	JSON_SAVE_BY_NAME("end", emitter.emitterArea.capsule.segment.diff);
	JSON_PARENT();
	JSON_SAVE_BY_NAME("radius", emitter.emitterArea.capsule.radius);
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
	JSON_SAVE_BY_NAME("rotate", emitter.rotate.rotate);
	JSON_ROOT();

	JSON_OBJECT("ScaleAnimation");
	SaveStartEnd(emitter.scale.range);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	SaveMinMax(emitter.velocity.range);
	JSON_ROOT();

	JSON_SAVE_BY_NAME("createParticleNum", emitter.createParticleNum);
	JSON_SAVE_BY_NAME("textureIndex", emitter.textureIndex);

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	LoadStartEnd(desc.emitter.scale.range);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	LoadMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	LoadStartEnd(desc.emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_LOAD_BY_NAME("rotate", desc.emitter.rotate.rotate);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	LoadMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_LOAD_BY_NAME("NumCreate", desc.numCreate);
	JSON_LOAD_BY_NAME("textureIndex", desc.emitter.textureIndex);

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::MeshEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/MeshParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	SaveStartEnd(desc.emitter.scale.range);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	SaveMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	SaveStartEnd(desc.emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_SAVE_BY_NAME("rotate", desc.emitter.rotate.rotate);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	SaveMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_SAVE_BY_NAME("NumCreate", desc.numCreate);

	JSON_SAVE_BY_NAME("textureIndex", desc.emitter.textureIndex);

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Load(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	LoadStartEnd(desc.emitter.scale.range);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	LoadMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	LoadStartEnd(desc.emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_LOAD_BY_NAME("rotate", desc.emitter.rotate.rotate);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	LoadMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_LOAD_BY_NAME("textureIndex", desc.emitter.textureIndex);

	JSON_CLOSE();
}

void GPUParticleShaderStructs::Save(const std::string name, GPUParticleShaderStructs::VertexEmitterDesc& desc) {
	JSON_OPEN("Resources/GPUParticle/VertexParticle/" + name + ".json");

	JSON_OBJECT("ScaleAnimation");
	SaveStartEnd(desc.emitter.scale.range);
	JSON_ROOT();

	JSON_OBJECT("ParticleLifeTime");
	SaveMinMax(desc.emitter.particleLifeSpan.range);
	JSON_ROOT();

	JSON_OBJECT("EmitterColor");
	SaveStartEnd(desc.emitter.color.range);
	JSON_ROOT();

	JSON_OBJECT("RotateAnimation");
	JSON_SAVE_BY_NAME("rotate", desc.emitter.rotate.rotate);
	JSON_ROOT();

	JSON_OBJECT("Velocity3D");
	SaveMinMax(desc.emitter.velocity.range);
	JSON_ROOT();

	JSON_SAVE_BY_NAME("textureIndex", desc.emitter.textureIndex);

	JSON_CLOSE();
}

GPUParticleShaderStructs::MeshEmitterDesc::MeshEmitterDesc() {
	buffer.Create(L"MeshEmitterBuffer", sizeof(MeshEmitter));
}

GPUParticleShaderStructs::VertexEmitterDesc::VertexEmitterDesc() {
	buffer.Create(L"VertexEmitterBuffer", sizeof(VertexEmitterDesc));
}
