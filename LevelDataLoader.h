#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Quaternion.h"

namespace LevelDataLoader {
	struct ObjectData {
		struct Transform {
			Vector3 translate;
			Quaternion rotate;
			Vector3 scale;
		} transform;
		struct Collider {
			Vector3 center;
			Quaternion rotate;
			Vector3 size;
		} collider;
	};
	void Load(const std::filesystem::path& path);
	

	std::unordered_map<std::string, ObjectData> objects_;
}