#pragma once
/**
 * @file JsonUtils.h
 * @brief Jsonのオーバーロードやマクロ
 */
#include <stdint.h>

#include <stdint.h>
#include <string>
#include <filesystem>
#include "../Externals/nlohmann/json.hpp"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"

void to_json(nlohmann::json& json, const bool& value);
void to_json(nlohmann::json& json, const int32_t& value);
void to_json(nlohmann::json& json, const uint32_t& value);
void to_json(nlohmann::json& json, const float& value);
void to_json(nlohmann::json& json, const Vector2& value);
void to_json(nlohmann::json& json, const Vector3& value);
void to_json(nlohmann::json& json, const Vector4& value);
void to_json(nlohmann::json& json, const Quaternion& value);
void to_json(nlohmann::json& json, const std::string& value);

void from_json(const nlohmann::json& json, bool& value);
void from_json(const nlohmann::json& json, int32_t& value);
void from_json(const nlohmann::json& json, uint32_t& value);
void from_json(const nlohmann::json& json, float& value);
void from_json(const nlohmann::json& json, Vector2& value);
void from_json(const nlohmann::json& json, Vector3& value);
void from_json(const nlohmann::json& json, Vector4& value);
void from_json(const nlohmann::json& json, Quaternion& value);
void from_json(const nlohmann::json& json, std::string& value);

namespace JsonHelper {

    bool Open(const std::filesystem::path& path);
    bool Object(const std::string& name = "");
    bool Parent();
    bool Close();


    void Save(const bool& value, const std::string& name);
    void Save(const int32_t& value, const std::string& name);
    void Save(const uint32_t& value, const std::string& name);
    void Save(const float& value, const std::string& name);
    void Save(const Vector2& value, const std::string& name);
    void Save(const Vector3& value, const std::string& name);
    void Save(const Vector4& value, const std::string& name);
    void Save(const Quaternion& value, const std::string& name);
    void Save(const std::string& value, const std::string& name);

    bool Load(bool& value, const std::string& name);
    bool Load(int32_t& value, const std::string& name);
    bool Load(uint32_t& value, const std::string& name);
    bool Load(float& value, const std::string& name);
    bool Load(Vector2& value, const std::string& name);
    bool Load(Vector3& value, const std::string& name);
    bool Load(Vector4& value, const std::string& name);
    bool Load(Quaternion& value, const std::string& name);
    bool Load(std::string& value, const std::string& name);
}
#ifndef STRINGIFY_HELPER
#define STRINGIFY_HELPER(x) #x
#endif
#ifndef STRINGIFY
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#endif

/// ファイルを開く
/// 必ず最初に呼び出す
/// x : ファイルのパス 
/// ファイルがもともとある時はTrue
#define JSON_OPEN(x) (JsonHelper::Open(x))
/// ファイルを閉じる
/// 必ず最後に呼び出す
/// ファイルがもともとある時はTrue
#define JSON_CLOSE() (JsonHelper::Close())
// Jsonのルートに戻る
#define JSON_ROOT() (JsonHelper::Object())
// Objectに入る
// x : Objectの名前
// Objectがある時True
#define JSON_OBJECT(x) (JsonHelper::Object(x))
// 引数に与えた変数の名前でセーブ
#define JSON_SAVE_BY_NAME(name, x) (JsonHelper::Save(x, name))
#define JSON_SAVE(x) (JsonHelper::Save(x, STRINGIFY(x)))
// 引数に与えた変数の名前でロード
// 読み込めたらTrue
#define JSON_LOAD_BY_NAME(name, x) (JsonHelper::Load(x, name))
#define JSON_LOAD(x) (JsonHelper::Load(x, STRINGIFY(x)))
#define JSON_PARENT() (JsonHelper::Parent())