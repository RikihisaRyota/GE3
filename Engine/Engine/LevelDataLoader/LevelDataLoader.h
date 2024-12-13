#pragma once
/**
 * @file LevelDataLoader
 * @brief レベルエディターに必要な物をまとめた
 */
#include <filesystem>
#include <string>
#include <optional>
#include <vector>

#include "../Externals/nlohmann/json.hpp"

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/WorldTransform.h"

namespace LevelDataLoader {
	// オブジェクトの座標構造体
	struct Transform {
		Vector3 translate;
		Quaternion rotate;
		Vector3 scale;
		static int32_t index;
		int32_t parent = -1;
	};

	// 当たり判定構造体
	struct Collider {
		Vector3 center;
		Quaternion rotate;
		Vector3 size;
	};

	// ゲームオブジェクト構造体
	struct GameObject {
		Transform transform;
		std::optional<LevelDataLoader::Collider> collider;
		std::string fileName;
		std::optional<std::string> objectName;
	};

	// ゲームオブジェクト構造体配列
	struct ObjectData {
		std::vector<GameObject> gameObject;
	};

	// ロード
	void Load(const std::filesystem::path& path);

	// セット
	void SetGameObject(const nlohmann::json& object, int32_t index = -1);

	extern ObjectData objectData_;
}