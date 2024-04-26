#pragma once

#include <string>
#include <map>
#include <optional>
#include <vector>

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Matrix4x4.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Model/Model.h"

namespace Animation {

	struct Joint {
		Model::QuaternionTransform transform;
		Matrix4x4 localMatrix;
		Matrix4x4 skeletonSpaceMatrix;
		std::string name;
		std::vector<int32_t> children;
		int32_t index;
		std::optional<int32_t> parent;
	};
	struct Skeleton {
		int32_t root;
		std::map<std::string, int32_t> jointMap;
		std::vector<Joint> joints;

		void CreateSkeleton(const Model::Node& rootNode);
		void Update();
	};

	int32_t CreateJoint(const Model::Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
}