#include "Animation.h"

#include <iostream>

#include <assert.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/DrawLine/DrawLine.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Graphics/RenderManager.h"

namespace Animation {
	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time) {
		assert(!keyframes.keyframe.empty());
		for (uint32_t index = 0; index < keyframes.keyframe.size() - 1; ++index) {
			uint32_t nextIndex = index + 1;
			// indexとnextIndexが範囲内か
			if (keyframes.keyframe[index].time <= time && time <= keyframes.keyframe[nextIndex].time) {
				// 範囲内に補正
				float t = (time - keyframes.keyframe[index].time) / (keyframes.keyframe[nextIndex].time - keyframes.keyframe[index].time);
				return Lerp(keyframes.keyframe[index].value, keyframes.keyframe[nextIndex].value, t);
			}
		}

		return (*keyframes.keyframe.rbegin()).value;
	}

	Quaternion CalculateValue(const AnimationCurve<Quaternion>& keyframes, float time) {
		assert(!keyframes.keyframe.empty());
		for (uint32_t index = 0; index < keyframes.keyframe.size() - 1; ++index) {
			uint32_t nextIndex = index + 1;
			// indexとnextIndexが範囲内か
			if (keyframes.keyframe[index].time <= time && time <= keyframes.keyframe[nextIndex].time) {
				// 範囲内に補正
				float t = (time - keyframes.keyframe[index].time) / (keyframes.keyframe[nextIndex].time - keyframes.keyframe[index].time);
				return Slerp(keyframes.keyframe[index].value, keyframes.keyframe[nextIndex].value, t);
			}
		}

		return (*keyframes.keyframe.rbegin()).value;
	}

	void ApplyAnimation(Skeleton& skeleton, const AnimationDesc& animation, float animationTime) {
		for (auto& joint : skeleton.joints) {
			if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
				const NodeAnimation& rootNodeAnimation = (*it).second;
				joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
				joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
				joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
			}
		}
	}

	std::vector<AnimationDesc> LoadAnimationFile(const std::filesystem::path& directoryPath) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(directoryPath.string(), 0);
		// アニメーションがない
		if (!scene) {
			std::cerr << "Error: Failed to load input file: " << importer.GetErrorString() << std::endl;
		assert(0);
		}
		assert(scene->mNumAnimations != 0);
		std::vector<AnimationDesc> animationDescs{};
		for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
			AnimationDesc desc{};
			aiAnimation* animationAssimp = scene->mAnimations[i];
			desc.name = scene->mAnimations[i]->mName.C_Str();
			desc.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);
			for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
				aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
				NodeAnimation& nodeAnimation = desc.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
				// translate
				for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
					aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
					KeyframeVector3 keyframe{};
					keyframe.time = float(keyAssimp.mTime / animationAssimp->mDuration);
					keyframe.value = { -keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
					nodeAnimation.translate.keyframe.emplace_back(keyframe);
				}
				// rotate
				for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
					aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
					KeyframeQuaternion keyframe{};
					keyframe.time = float(keyAssimp.mTime / animationAssimp->mDuration);
					// 右手から左手にするためにyとz反転
					keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y,-keyAssimp.mValue.z,  keyAssimp.mValue.w };
					nodeAnimation.rotate.keyframe.emplace_back(keyframe);
				}
				// scale
				for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
					aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
					KeyframeVector3 keyframe{};
					keyframe.time = float(keyAssimp.mTime / animationAssimp->mDuration);
					keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
					nodeAnimation.scale.keyframe.emplace_back(keyframe);
				}
			}
			animationDescs.emplace_back(desc);
		}
		return animationDescs;
	}

	void AnimationDesc::Update(WorldTransform& worldTransform, bool isLoop, const ModelHandle& modelHandle, uint32_t children) {
		animationTime += 1.0f / duration;
		animationTime = std::clamp(animationTime, 0.0f, duration);
		if (isLoop) {
			animationTime = std::fmod(animationTime, duration);
		}
		NodeAnimation& rootAnimation = nodeAnimations[ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(children)->rootNode.name];
		worldTransform.translate = CalculateValue(rootAnimation.translate, animationTime);
		worldTransform.rotate = CalculateValue(rootAnimation.rotate, animationTime);
		worldTransform.scale = CalculateValue(rootAnimation.scale, animationTime);
	}

	AnimationHandle Animation::GetAnimationHandle(const std::string& name) {
		for (uint32_t index = 0; auto & animation : animations) {
			if (animation.name == name) {
				return index;
			}
			index++;
		}
		assert(0);
		return 0;
	}

	void Animation::Initialize(const ModelHandle& modelHandle) {
		animations = LoadAnimationFile(ModelManager::GetInstance()->GetModel(modelHandle).GetPath());
		skeleton.CreateSkeleton(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->rootNode);
		skinCluster.CreateSkinCluster(skeleton, modelHandle);

		debugBoxModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Box1x1/box1x1.gltf");
		for (auto& joint : skeleton.joints) {
			debugBox_.emplace_back(WorldTransform());
			debugBox_.back().Initialize();
		}

	}

	void Animation::Initialize(const std::filesystem::path& path, const ModelHandle& modelHandle) {
		animations = LoadAnimationFile(path);
		skeleton.CreateSkeleton(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->rootNode);
		skinCluster.CreateSkinCluster(skeleton, modelHandle);

		debugBoxModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Box1x1/box1x1.gltf");
		for (auto& joint : skeleton.joints) {
			debugBox_.emplace_back(WorldTransform());
			debugBox_.back().Initialize();
		}
	}

	void Animation::Update(const AnimationHandle& handle, float time, CommandContext& commandContext, const ModelHandle& modelHandle) {
		ApplyAnimation(skeleton, animations.at(handle), time);
		skeleton.Update();
		skinCluster.Update(skeleton, commandContext, modelHandle);
	}

	void Animation::DrawLine(const WorldTransform& worldTransform) {
		auto drawLine = DrawLine::GetInstance();
		for (auto& joint : skeleton.joints) {
			drawLine->SetLine(MakeTranslateMatrix(Mul(joint.skeletonSpaceMatrix, worldTransform.matWorld)), { 0.0f,1.0f,0.0f,1.0f });

			if (joint.parent) {
				drawLine->SetLine(MakeTranslateMatrix(Mul(skeleton.joints.at(*joint.parent).skeletonSpaceMatrix, worldTransform.matWorld)), { 0.0f,1.0f,0.0f,1.0f });
			}
			else {
				drawLine->SetLine(MakeTranslateMatrix(Mul(joint.skeletonSpaceMatrix, worldTransform.matWorld)), { 0.0f,1.0f,0.0f,1.0f });
			}
		}
	}
	void Animation::DrawBox(const WorldTransform& worldTransform, const ViewProjection& viewProjection) {
		auto drawBox = ModelManager::GetInstance();
		auto renderManager = RenderManager::GetInstance();
		for (uint32_t i = 0; auto & joint : skeleton.joints) {

			debugBox_.at(i).matWorld = Mul(joint.skeletonSpaceMatrix, worldTransform.matWorld);
			debugBox_.at(i).TransferMatrix();
			drawBox->Draw(debugBox_.at(i), viewProjection, debugBoxModelHandle_, renderManager->GetCommandContext());
			i++;
		}
	}

}