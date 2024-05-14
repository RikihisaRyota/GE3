#pragma once
#include <cstdint>

namespace CollisionAttribute {
    // これプレイヤーかそれ以外だけでいいんじゃね？
    const uint32_t Player = 0b1;
    const uint32_t Boss = 0b10;
}