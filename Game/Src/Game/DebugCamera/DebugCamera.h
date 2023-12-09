#pragma once
#include "../../Game/Engine/Math/ViewProjection.h"


class DebugCamera {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	DebugCamera();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(ViewProjection* viewProjection);
public:
	static constexpr double pi = 3.14159265358979323846;
};

