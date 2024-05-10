#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/Model.h"
#include "Engine/Animation/Skeleton.h"
#include "Engine/Animation/Skinning.h"

namespace Animation {
	template <typename tValue>
	struct  Keyframe {
		tValue value;
		float time;
	};

	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeQuaternion = Keyframe<Quaternion>;

	template <typename tValue>
	struct AnimationCurve {
		std::vector<Keyframe<tValue>> keyframe;
	};

	struct NodeAnimation {
		AnimationCurve<Vector3> translate;
		AnimationCurve<Quaternion> rotate;
		AnimationCurve<Vector3> scale;
	};

	struct AnimationDesc {
		void LoadAnimationFile(const std::filesystem::path& directoryPath);
		void Update(WorldTransform& worldTransform, bool isLoop, const ModelHandle& modelHandle, uint32_t children);
		std::map<std::string, NodeAnimation> nodeAnimations;
		// 単位は秒
		float duration;
		float animationTime;
		std::string name;
	};

	struct Animation {
		AnimationDesc animation;
		Skeleton skeleton;
		SkinCluster skinCluster;
		void Initialize(const ModelHandle& modelHandle);
		void Update(float time);
		void DrawLine(const WorldTransform& worldTransform);
		void DrawBox(const WorldTransform& worldTransform, const ViewProjection& viewProjection);
		std::vector<WorldTransform> debugBox_;
		ModelHandle debugBoxModelHandle_;
	};

	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	Quaternion CalculateValue(const AnimationCurve<Quaternion>& keyframes, float time);
	void ApplyAnimation(Skeleton& skeleton, const AnimationDesc& animation, float animationTime);
}
