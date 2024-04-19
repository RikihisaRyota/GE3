#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/Model.h"
#include "Engine/Animation/Skeleton.h"

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

struct Animation {
	void LoadAnimationFile(const std::filesystem::path& directoryPath);
	void Update(WorldTransform& worldTransform, bool isLoop, const ModelHandle& modelHandle, uint32_t children);
	std::map<std::string, NodeAnimation> nodeAnimations;
	// 単位は秒
	float duration;
	float animationTime;
	std::string name;
};

Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
Quaternion CalculateValue(const AnimationCurve<Quaternion>& keyframes, float time);
void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);
