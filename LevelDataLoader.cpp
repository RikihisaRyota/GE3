#include "LevelDataLoader.h"

#include <assert.h>
#include <fstream>

#include "Externals/nlohmann/json.hpp"

namespace LevelDataLoader {
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
		for (auto& object : deserialized) {
			assert(object.contains("type"));


		}
	}
}
