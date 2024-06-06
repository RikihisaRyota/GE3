#pragma once

#include <filesystem>
#include <string>
#include <optional>
#include <vector>

#include "../Externals/nlohmann/json.hpp"

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/WorldTransform.h"

namespace LevelDataLoader {

	struct Transform {
		Vector3 translate;
		Quaternion rotate;
		Vector3 scale;
		static int32_t index;
		int32_t parent = -1;
	};

	struct Collider {
		Vector3 center;
		Quaternion rotate;
		Vector3 size;
	};

	struct GameObject {
		Transform transform;
		std::optional<LevelDataLoader::Collider> collider;
		std::string fileName;
	};

	struct ObjectData {
		std::vector<GameObject> gameObject;
	};
	void Load(const std::filesystem::path& path);

	void SetGameObject(const nlohmann::json& object, int32_t index = -1);

	extern ObjectData objectData_;
}