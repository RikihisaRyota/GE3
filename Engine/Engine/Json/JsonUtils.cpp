#include "JsonUtils.h"

#include <cassert>
#include <fstream>
#include <stack>

//void to_json(nlohmann::json& json, const int& value) {
//	json = value;
//}

void to_json(nlohmann::json& json, const bool& value) {
	json = value;
}

void to_json(nlohmann::json& json, const int32_t& value) {
	json = value;
}

void to_json(nlohmann::json& json, const uint32_t& value) {
	json = value;
}

void to_json(nlohmann::json& json, const float& value) {
	json = value;
}

void to_json(nlohmann::json& json, const Vector2& value) {
	json = nlohmann::json::array({ value.x, value.y });
}

void to_json(nlohmann::json& json, const Vector3& value) {
	json = nlohmann::json::array({ value.x, value.y, value.z });
}

void to_json(nlohmann::json& json, const Vector4& value) {
	json = nlohmann::json::array({ value.x, value.y, value.z, value.w });
}

void to_json(nlohmann::json& json, const Quaternion& value) {
	json = nlohmann::json::array({ value.x, value.y, value.z, value.w });
}

void to_json(nlohmann::json& json, const std::string& value) {
	json = value;
}

void from_json(const nlohmann::json& json, bool& value) {
	value = json.get<bool>();
}

// int32_t 型に対する from_json の実装
void from_json(const nlohmann::json& json, int32_t& value) {
	if (!json.is_number_integer()) {
		throw std::invalid_argument("JSON value is not a 32-bit integer");
	}
	value = json.get<int32_t>();
}

// uint32_t 型に対する from_json の実装
void from_json(const nlohmann::json& json, uint32_t& value) {
	if (!json.is_number_unsigned()) {
		throw std::invalid_argument("JSON value is not an unsigned 32-bit integer");
	}
	value = json.get<uint32_t>();
}

void from_json(const nlohmann::json& json, float& value) {
	if (!json.is_number_float() && !json.is_number_integer()) {
		throw std::invalid_argument("JSON value is not a number");
	}
	value = json.get<float>();
}

void from_json(const nlohmann::json& json, Vector2& value) {
	assert(json.is_array() && json.size() == 2);
	value = Vector2(json.at(0), json.at(1));
}

void from_json(const nlohmann::json& json, Vector3& value) {
	assert(json.is_array() && json.size() == 3);
	value = Vector3(json.at(0), json.at(1), json.at(2));
}

void from_json(const nlohmann::json& json, Vector4& value) {
	assert(json.is_array() && json.size() == 4);
	value = Vector4(json.at(0), json.at(1), json.at(2), json.at(3));
}

void from_json(const nlohmann::json& json, Quaternion& value) {
	assert(json.is_array() && json.size() == 4);
	value = Quaternion(json.at(0), json.at(1), json.at(2), json.at(3));
}

void from_json(const nlohmann::json& json, std::string& value) {
	value = json.get<std::string>();
}

namespace JsonHelper {

	static std::filesystem::path openPath;
	static nlohmann::json root;
	static nlohmann::json* target;
	static std::stack<nlohmann::json*> targetStack;

	bool Open(const std::filesystem::path& path) {
		assert(!target);
		root.clear();
		if (std::filesystem::exists(path)) {
			std::ifstream file;
			file.open(path);
			assert(file.is_open());
			file >> root;
			file.close();
		}
		openPath = path;
		target = &root;
		while (!targetStack.empty()) {
			targetStack.pop();
		}
		return !root.empty();
	}

	bool Object(const std::string& name) {
		assert(target);
		bool contains = false;
		if (name.empty()) {
			target = &root;
		}
		else {
			contains = target->contains(name);
			if (!contains) {
				target->emplace(name, nlohmann::json::object());
			}
			targetStack.push(target);
			target = &target->at(name);
		}
		return contains;
	}

	bool Parent() {
		if (targetStack.empty()) {
			return false;
		}
		target = targetStack.top();
		targetStack.pop();
		return true;
	}

	bool Close() {
		assert(!openPath.empty());
		assert(target);
		bool exists = std::filesystem::exists(openPath);
		if (!root.empty()) {
			std::filesystem::create_directories(openPath.parent_path());
			std::ofstream file;
			file.open(openPath);
			assert(file.is_open());
			file << std::setw(4) << root << std::endl;
			file.close();
		}
		openPath.clear();
		root.clear();
		target = nullptr;
		while (!targetStack.empty()) {
			targetStack.pop();
		}
		return exists;
	}

	void Save(const bool& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const int32_t& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const uint32_t& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const float& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const Vector2& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const Vector3& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const Vector4& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const Quaternion& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	void Save(const std::string& value, const std::string& name) {
		to_json((*target)[name], value);
	}

	bool Load(bool& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// bool型かどうかの確認
		assert(iter->is_boolean());
		from_json(*iter, value);
		return true;
	}

	bool Load(int32_t& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 整数型
		assert(iter->is_number_integer());
		from_json(*iter, value);
		return true;
	}

	bool Load(uint32_t& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 符号なし整数型
		assert(iter->is_number_unsigned());
		from_json(*iter, value);
		return true;
	}

	bool Load(float& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 符号なし整数型
		assert(iter->is_number_float());
		from_json(*iter, value);
		return true;
	}

	bool Load(Vector2& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 配列型かつ要素2
		assert(iter->is_array() && iter->size() == 2);
		from_json(*iter, value);
		return true;
	}

	bool Load(Vector3& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 配列型かつ要素3
		assert(iter->is_array() && iter->size() == 3);
		from_json(*iter, value);
		return true;
	}

	bool Load(Vector4& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 配列型かつ要素4
		assert(iter->is_array() && iter->size() == 4);
		from_json(*iter, value);
		return true;
	}

	bool Load(Quaternion& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 配列型かつ要素4
		assert(iter->is_array() && iter->size() == 4);
		from_json(*iter, value);
		return true;
	}

	bool Load(std::string& value, const std::string& name) {
		auto iter = target->find(name);
		// ファイルにデータがない
		if (iter == target->end()) {
			return false;
		}
		// 配列型かつ要素4
		assert(iter->is_string());
		from_json(*iter, value);
		return true;
	}

}