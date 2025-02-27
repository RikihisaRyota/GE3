#pragma once
/**
 * @file CollisionAttribute
 * @brief 当たり判定のマスク
 */
#include <cstdint>

namespace CollisionAttribute {
    const uint32_t Player =         0b1;
    const uint32_t PlayerBullet =   0b10;
    const uint32_t Boss =           0b100;
    const uint32_t GameObject =     0b1000;
    const uint32_t ParticleField =  0b10000;
}