#include "Skeleton.h"

#include "Engine/Model/Model.h"
#include "Engine/Math/MyMath.h"

namespace Engine {
	namespace Animation {
		int32_t CreateJoint(const Model::Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
			Joint joint{};
			joint.name = node.name;
			joint.localMatrix = node.localMatrix;
			joint.skeletonSpaceMatrix = MakeIdentity4x4();
			joint.transform = node.transform;
			joint.index = int32_t(joints.size());
			joint.parent = parent;
			joints.emplace_back(joint);
			for (auto& child : node.children) {
				int32_t childIndex = CreateJoint(child, joint.index, joints);
				joints.at(joint.index).children.emplace_back(childIndex);
			}
			return joint.index;
		}

		void Skeleton::CreateSkeleton(const Model::Node& rootNode) {
			root = CreateJoint(rootNode, {}, joints);
			for (const Joint& joint : joints) {
				jointMap.emplace(joint.name, joint.index);
			}
			Update();
		}

		void Skeleton::Update() {
			for (auto& joint : joints) {
				joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
				if (joint.parent) {
					joint.skeletonSpaceMatrix = joint.localMatrix * joints.at(*joint.parent).skeletonSpaceMatrix;
				}
				else {
					joint.skeletonSpaceMatrix = joint.localMatrix;
				}
			}
		}
	}
}