#pragma once
/**
 * @file Animation.h
 * @brief AnimationDataを管理
 */
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
		void Update(WorldTransform& worldTransform, bool isLoop, const ModelHandle& modelHandle, uint32_t children);
		std::map<std::string, NodeAnimation> nodeAnimations;
		// 単位は秒
		float duration;
		float animationTime;
		std::string name;
	};
	std::vector<AnimationDesc> LoadAnimationFile(const std::filesystem::path& directoryPath);

	class AnimationHandle {
	public:
		static const ModelHandle kMaxModeHandle;

		// デフォルトのコンストラクタを追加
		AnimationHandle() = default;
		// size_t からの暗黙の型変換を行う関数を追加
		AnimationHandle(size_t value) : index_(value) {}

		operator size_t () const { return index_; }
		bool IsValid() const { return index_ != ((size_t)-1); }
	private:
		size_t index_ = ((size_t)-1);
	};


	struct Animation {
		std::vector<AnimationDesc> animations;
		Skeleton skeleton;
		SkinCluster skinCluster;
		// ゲッター
		AnimationHandle GetAnimationHandle(const std::string& name);

		// modelHandleからの初期化
		void Initialize(const ModelHandle& modelHandle);
		// pathからの初期化
		void Initialize(const std::filesystem::path& path, const ModelHandle& modelHandle);
		// アップデート
		void Update(const AnimationHandle& handle, float time, CommandContext& commandContext, const ModelHandle& modelHandle);
		// アニメーション遷移アップデート
		void Update(const AnimationHandle& pre, float fromTime, const AnimationHandle& current, float toTime, float time, CommandContext& commandContext, const ModelHandle& modelHandle);
		// ボーン表示
		void DrawLine(const WorldTransform& worldTransform);
	};

	// キーフレーム取得
	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	Quaternion CalculateValue(const AnimationCurve<Quaternion>& keyframes, float time);
	void ApplyAnimation(Skeleton& skeleton, const AnimationDesc& animation, float animationTime);
	void ApplyAnimationTransition(Skeleton& skeleton, const AnimationDesc& from, float fromTime, const AnimationDesc& to, float toTime, float animationTime);
}
