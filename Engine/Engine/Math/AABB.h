#pragma once
#include "Vector3.h"
struct AABB {
	Vector3 center_; //!< 真ん中
	Vector3 min_; //!< 最小点
	Vector3 max_; //!< 最大点
};