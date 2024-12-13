#pragma once
/**
 * @file FollowCamera.h
 * @brief フォローカメラ
 */
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"

class FollowCamera {
public:
	void Initialize();

	void Update();

	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
	void SetTarget(const WorldTransform* target) { target_ = target; }

	void DrawImGui();
private:
	void GamePad();
	void Keyboard();
	void RotateUpdate();

	// 回転速度
	const float kRotateSpeedX = 0.000001f;
	const float kRotateSpeedY = 0.000002f;
	// 上下方向の回転のデッドゾーン
	const float kDeadZoneRotateMin = -5.0f;
	const float kDeadZoneRotateMax = 70.0f;
	// キーボードカメラスピード
	const float kKeyboardCameraSpeed_Y = 10000.0f;
	const float kKeyboardCameraSpeed_X = 15000.0f;

	ViewProjection* viewProjection_;
	const WorldTransform* target_ = nullptr;

	Vector2 destinationAngle_;
	Vector3 interTarget_;

	Vector3 offset_;
};