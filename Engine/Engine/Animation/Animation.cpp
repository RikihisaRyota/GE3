#include "Animation.h"

#include <assert.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/DrawLine/DrawLine.h"
#include "Engine/Math/MyMath.h"


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

	void AnimationDesc::LoadAnimationFile(const std::filesystem::path& directoryPath) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(directoryPath.string(), 0);
		// アニメーションがない
		assert(scene->mNumAnimations != 0);
		aiAnimation* animationAssimp = scene->mAnimations[0];
		duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);
		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
			aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
			// translate
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyframeVector3 keyframe{};
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyframe.value = { -keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
				nodeAnimation.translate.keyframe.emplace_back(keyframe);
			}
			// rotate
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
				aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyframeQuaternion keyframe{};
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				// 右手から左手にするためにyとz反転
				keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y,-keyAssimp.mValue.z,  keyAssimp.mValue.w };
				nodeAnimation.rotate.keyframe.emplace_back(keyframe);
			}
			// scale
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyframeVector3 keyframe{};
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
				nodeAnimation.scale.keyframe.emplace_back(keyframe);
			}
		}
	}

	void AnimationDesc::Update(WorldTransform& worldTransform, bool isLoop, const ModelHandle& modelHandle, uint32_t children) {
		animationTime += 1.0f / 60.0f;
		animationTime = std::clamp(animationTime, 0.0f, duration);
		if (isLoop) {
			animationTime = std::fmod(animationTime, duration);
		}
		NodeAnimation& rootAnimation = nodeAnimations[ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(children)->rootNode.name];
		worldTransform.translate = CalculateValue(rootAnimation.translate, animationTime);
		worldTransform.rotate = CalculateValue(rootAnimation.rotate, animationTime);
		worldTransform.scale = CalculateValue(rootAnimation.scale, animationTime);
	}

	void Animation::Initialize(const ModelHandle& modelHandle) {
		animation.LoadAnimationFile(ModelManager::GetInstance()->GetModel(modelHandle).GetPath());
		skeleton.CreateSkeleton(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->rootNode);
		skinCluster.CreateSkinCluster(skeleton, modelHandle);
	}

	void Animation::Update(float time) {
		ApplyAnimation(skeleton, animation, time);
		skeleton.Update();
		skinCluster.Update(skeleton);
	}

	void Animation::Draw(const WorldTransform& worldTransform) {
		auto drawLine = DrawLine::GetInstance();
		for (auto& joint : skeleton.joints) {
			drawLine->SetLine(Transform(joint.transform.translate, Mul(joint.skeletonSpaceMatrix, worldTransform.matWorld)), { 0.0f,1.0f,0.0f,1.0f });

			if (joint.parent) {
				drawLine->SetLine(Transform(skeleton.joints.at(*joint.parent).transform.translate, Mul(joint.skeletonSpaceMatrix, worldTransform.matWorld)), { 0.0f,1.0f,0.0f,1.0f });
			}
			else {
				drawLine->SetLine(Transform(joint.transform.translate, Mul(joint.skeletonSpaceMatrix, worldTransform.matWorld)), { 0.0f,1.0f,0.0f,1.0f });
			}
		}
	}

}