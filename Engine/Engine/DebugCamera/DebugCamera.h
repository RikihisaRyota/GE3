#pragma once
/**
 * @file DebugCamera.h
 * @brief デバックカメラ
 */

#include "Engine/Math/ViewProjection.h"

class DebugCamera {
public:
	void Update(ViewProjection* viewProjection);
	// デバックカメラのonoff
	bool GetIsDebugCamera() { return  isDebugCamera_; }
private:
	bool isDebugCamera_;
};

