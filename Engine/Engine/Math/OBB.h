#pragma once
#include "Vector3.h"
struct OBB {
	Vector3 center;          //!< 中心点
	Vector3 orientations[3]; //!< 座標軸。正規化・直交必須
	Vector3 size;            //!< 座標方向の長さの半分。中心から面までの距離
};