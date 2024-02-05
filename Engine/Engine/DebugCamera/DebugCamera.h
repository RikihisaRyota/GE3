#pragma once
#include "Engine/Math/ViewProjection.h"


class DebugCamera {
public:
	void Update(ViewProjection* viewProjection);

	bool GetIsDebugCamera() { return  isDebugCamera_; }
private:
	bool isDebugCamera_;
};

