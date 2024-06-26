#include "JsonUtils.h"

#include <cassert>
#include <fstream>

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

void to_json(nlohmann::json& json, const GPUParticleShaderStructs::UintMinMax& value) {
	to_json(json["Min"], value.min);
	to_json(json["Max"], value.max);
}

void to_json(nlohmann::json& json, const GPUParticleShaderStructs::Vector3MinMax& value) {
	to_json(json["Min"], value.min);
	to_json(json["Max"], value.max);
}

void to_json(nlohmann::json& json, const GPUParticleShaderStructs::Vector4MinMax& value) {
	to_json(json["Min"], value.min);
	to_json(json["Max"], value.max);
}

void to_json(nlohmann::json& json, const GPUParticleShaderStructs::Vector3StartEnd& value) {
	to_json(json["Start"], value.start);
	to_json(json["End"], value.end);
}

void to_json(nlohmann::json& json, const GPUParticleShaderStructs::Vector4StartEnd& value) {
	to_json(json["Start"], value.start);
	to_json(json["End"], value.end);
}

void to_json(nlohmann::json& json, const GPUParticleShaderStructs::EmitterForCPU& value) {
	to_json(json["EmitterArea"]["EmitterAABB"], value.emitterArea.aabb.area);
	to_json(json["EmitterArea"]["EmitterSphere"]["radius"], value.emitterArea.sphere.radius);
	to_json(json["EmitterArea"]["EmitterCapsule"]["EmitterSegment"]["start"], value.emitterArea.capsule.segment.origin);
	to_json(json["EmitterArea"]["EmitterCapsule"]["EmitterSegment"]["end"], value.emitterArea.capsule.segment.diff);
	to_json(json["EmitterArea"]["EmitterCapsule"]["EmitterSegment"]["radius"], value.emitterArea.capsule.radius);
	to_json(json["EmitterArea"]["position"], value.emitterArea.position);
	to_json(json["EmitterArea"]["type"], value.emitterArea.type);
	to_json(json["ScaleAnimation"], value.scale.range);
	to_json(json["RotateAnimation"]["rotate"], value.rotate.rotate);
	to_json(json["Velocity3D"], value.velocity.range);
	to_json(json["EmitterColor"], value.color.range);
	to_json(json["EmitterFrequency"]["interval"], value.frequency.interval);
	to_json(json["EmitterFrequency"]["isLoop"], value.frequency.isLoop);
	to_json(json["EmitterFrequency"]["emitterLife"], value.frequency.emitterLife);
	to_json(json["ParticleLifeSpan"], value.particleLifeSpan.range);
	to_json(json["textureIndex"], value.particleLifeSpan.range);
	to_json(json["createParticleNum"], value.particleLifeSpan.range);
}

void from_json(const nlohmann::json& json, bool& value) {
	value = json.get<bool>();
}

// int32_t 型に対する from_json の実装
void from_json(const nlohmann::json& json, int32_t& value) {
	if (!json.is_number_integer()) {
		throw std::invalid_argument("JSON value is not a 32-bit integer");
		assert(0);
	}
	value = json.get<int32_t>();
}

// uint32_t 型に対する from_json の実装
void from_json(const nlohmann::json& json, uint32_t& value) {
	if (!json.is_number_unsigned()) {
		throw std::invalid_argument("JSON value is not an unsigned 32-bit integer");
		assert(0);
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
void from_json(const nlohmann::json& json, GPUParticleShaderStructs::UintMinMax& value) {
	from_json(json["Min"], value.min);
	from_json(json["Max"], value.max);
}
void from_json(const nlohmann::json& json, GPUParticleShaderStructs::Vector3MinMax& value) {
	from_json(json["Min"], value.min);
	from_json(json["Max"], value.max);
}
void from_json(const nlohmann::json& json, GPUParticleShaderStructs::Vector4MinMax& value) {
	from_json(json["Min"], value.min);
	from_json(json["Max"], value.max);
}
void from_json(const nlohmann::json& json, GPUParticleShaderStructs::Vector3StartEnd& value) {
	from_json(json["Start"], value.start);
	from_json(json["End"], value.end);
}
void from_json(const nlohmann::json& json, GPUParticleShaderStructs::Vector4StartEnd& value) {
	from_json(json["Start"], value.start);
	from_json(json["End"], value.end);
}
void from_json(const nlohmann::json& json, GPUParticleShaderStructs::EmitterForCPU& value) {
	from_json(json["EmitterArea"]["EmitterAABB"], value.emitterArea.aabb.area);
	from_json(json["EmitterArea"]["EmitterSphere"]["radius"], value.emitterArea.sphere.radius);
	from_json(json["EmitterArea"]["EmitterCapsule"]["EmitterSegment"]["start"], value.emitterArea.capsule.segment.origin);
	from_json(json["EmitterArea"]["EmitterCapsule"]["EmitterSegment"]["end"], value.emitterArea.capsule.segment.diff);
	from_json(json["EmitterArea"]["EmitterCapsule"]["EmitterSegment"]["radius"], value.emitterArea.capsule.radius);
	from_json(json["EmitterArea"]["position"], value.emitterArea.position);
	from_json(json["EmitterArea"]["type"], value.emitterArea.type);
	from_json(json["ScaleAnimation"], value.scale.range);
	from_json(json["RotateAnimation"]["rotate"], value.rotate.rotate);
	from_json(json["Velocity3D"], value.velocity.range);
	from_json(json["EmitterColor"], value.color.range);
	from_json(json["EmitterFrequency"]["interval"], value.frequency.interval);
	from_json(json["EmitterFrequency"]["isLoop"], value.frequency.isLoop);
	from_json(json["EmitterFrequency"]["emitterLife"], value.frequency.emitterLife);
	from_json(json["ParticleLifeSpan"], value.particleLifeSpan.range);
	from_json(json["textureIndex"], value.particleLifeSpan.range);
	from_json(json["createParticleNum"], value.particleLifeSpan.range);
}
namespace JsonHelper {

	static std::filesystem::path openPath;
	static nlohmann::json root;
	static nlohmann::json* target;

	bool Open(const std::filesystem::path& path) {
		assert(!target);
		// クリア
		root.clear();
		if (std::filesystem::exists(path)) {
			// ファイルを開く
			std::ifstream file;
			file.open(path);
			// 開けなかった
			assert(file.is_open());
			// 読み込む
			file >> root;
			file.close();
		}
		openPath = path;
		target = &root;
		// ファイルがあったか
		return !root.empty();
	}

	bool Object(const std::string& name) {
		assert(target);

		bool contains = false;

		// rootに挿入する
		if (name.empty()) {
			target = &root;
		}
		// オブジェクトを追加
		else {
			contains = target->contains(name);
			// まだObjectがない
			if (!contains) {
				// 新しいオブジェクトを追加
				target->emplace(name, nlohmann::json::object());
			}
			// 追加したオブジェクトが次のターゲット
			target = &target->at(name);
		}
		return contains;
	}

	bool Close() {
		assert(!openPath.empty());
		assert(target);
		bool exists = std::filesystem::exists(openPath);
		if (!root.empty()) {
			// ディレクトリを作成しとく
			std::filesystem::create_directories(openPath.parent_path());
			// ファイルを開く
			std::ofstream file;
			file.open(openPath);
			// 開けなかった
			assert(file.is_open());
			// 書き込む
			file << std::setw(4) << root << std::endl;
			file.close();
		}
		// 消す
		openPath.clear();
		root.clear();
		target = nullptr;
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