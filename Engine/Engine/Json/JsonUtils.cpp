#include "JsonUtils.h"

#include <cassert>

void to_json(nlohmann::json& json, const Vector2& value) {
	json = nlohmann::json{ { value.x, value.y } };
}

void to_json(nlohmann::json& json, const Vector3& value) {
	json = nlohmann::json{ { value.x, value.y, value.z } };
}

void to_json(nlohmann::json& json, const Vector4& value) {
	json = nlohmann::json{ { value.x, value.y, value.z, value.w } };
}

void to_json(nlohmann::json& json, const Quaternion& value) {
	json = nlohmann::json{ { value.x, value.y, value.z, value.w } };
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
