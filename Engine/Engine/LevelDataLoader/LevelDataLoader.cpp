#include "LevelDataLoader.h"

#include <assert.h>
#include <fstream>

#include "../Json/JsonUtils.h"

namespace LevelDataLoader {
	ObjectData objectData_;
	int32_t Transform::index = -1;
	void Load(const std::filesystem::path& path) {
		std::ifstream file;

		file.open(path);
		if (file.fail()) {
			assert(0);
		}

		nlohmann::json deserialized;

		// 解凍
		file >> deserialized;

		// 正しいレベルデータファイル化チェック
		assert(deserialized.is_object());
		assert(deserialized.contains("name"));
		assert(deserialized["name"].is_string());

		// "name"を文字列として取得
		std::string name = deserialized["name"].get<std::string>();
		// 正しいレベルデータファイルかチェック
		assert(name.compare("scene") == 0);

		// "objects"の全オブジェクトの走査
		for (auto& object : deserialized["objects"]) {
			assert(object.contains("type"));

			// 種別を取得
			std::string type = object["type"].get<std::string>();

			// Mesh
			if (type.compare("MESH")==0) {
				LevelDataLoader::SetGameObject(object);
			}
		}
	}
	void SetGameObject(const nlohmann::json& object, int32_t parent) {
		// 要素追加
		objectData_.gameObject.emplace_back(GameObject{});
		auto& gameObject = objectData_.gameObject.back();
		gameObject.transform.index++;
		if (parent != -1) {
			gameObject.transform.parent = parent;
		}
		if (object.contains("file_name")) {
			gameObject.fileName = object["file_name"];
		}
		auto& transform = object["transform"];
		gameObject.transform.translate.x = float(transform["translation"][0]);
		gameObject.transform.translate.y = float(transform["translation"][2]);
		gameObject.transform.translate.z = float(transform["translation"][1]);

		//gameObject.transform.rotate.x = -float(transform["rotation"][0]);
		//gameObject.transform.rotate.y = -float(transform["rotation"][2]);
		//gameObject.transform.rotate.z = -float(transform["rotation"][1]);

		gameObject.transform.rotate.x = float(transform["rotation"][1]);
		gameObject.transform.rotate.y = float(transform["rotation"][3]);
		gameObject.transform.rotate.z = float(transform["rotation"][2]);
		gameObject.transform.rotate.w = float(transform["rotation"][0]);

		gameObject.transform.scale.x = float(transform["scaling"][0]);
		gameObject.transform.scale.y = float(transform["scaling"][2]);
		gameObject.transform.scale.z = float(transform["scaling"][1]);

		if (object.contains("collider")) {
			auto& collider = object["collider"];
			gameObject.collider = LevelDataLoader::Collider{};
			gameObject.collider->center.x = float(collider["center"][0]);
			gameObject.collider->center.y = float(collider["center"][2]);
			gameObject.collider->center.z = float(collider["center"][1]);

			gameObject.collider->rotate.x = -float(collider["rotate"][1]);
			gameObject.collider->rotate.y = -float(collider["rotate"][3]);
			gameObject.collider->rotate.z = -float(collider["rotate"][2]);
			gameObject.collider->rotate.w = -float(collider["rotate"][0]);

			gameObject.collider->size.x = float(collider["size"][0]);
			gameObject.collider->size.y = float(collider["size"][2]);
			gameObject.collider->size.z = float(collider["size"][1]);
		}
		if (object.contains("children")) {
			for (auto& child : object["children"]) {
				SetGameObject(child, gameObject.transform.index);
			}
		}
	}
}
