#pragma once

#include "../Externals/nlohmann/json.hpp"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Quaternion.h"

void to_json(nlohmann::json& json, const Vector2& value);
void to_json(nlohmann::json& json, const Vector3& value);
void to_json(nlohmann::json& json, const Vector4& value);
void to_json(nlohmann::json& json, const Quaternion& value);

void from_json(const nlohmann::json& json, Vector2& value);
void from_json(const nlohmann::json& json, Vector3& value);
void from_json(const nlohmann::json& json, Vector4& value);
void from_json(const nlohmann::json& json, Quaternion& value);