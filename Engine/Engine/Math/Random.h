#pragma once
/**
 * @file Random.h 
 * @brief ランダムを生成する
 */
#include <cstdint>
#include <random>

namespace Random {

    class RandomNumberGenerator {
    public:
        RandomNumberGenerator(uint32_t seed = 0) : engine_((seed == 0) ? seedGenerator_() : seed) {}
        // SeedSet
        void SetSeed(uint32_t seed) { engine_.seed(seed); }

        // 加減上限付きintランダム
        int32_t NextIntRange(int32_t min, int32_t max) { return std::uniform_int_distribution<int32_t>(min, max)(engine_); }
        // 加減上限無しintランダム
        int32_t NextIntLimit() { return NextIntRange(INT_MIN, INT_MAX); }

        // 加減上限ありuintランダム
        uint32_t NextUIntRange(uint32_t min, uint32_t max) { return std::uniform_int_distribution<uint32_t>(min, max)(engine_); }
        // 加減上限無しuintランダム
        uint32_t NextUIntLimit() { return NextUIntRange(0u, UINT_MAX); }

        // 加減上限ありfloatランダム
        float NextFloatRange(float min, float max) { return std::uniform_real_distribution<float>(min, max)(engine_); }
        // 加減上限無しfloatランダム
        float NextFloatLimit() { return NextFloatRange(FLT_MIN, FLT_MAX); }
        // 0.0f~1.0f,floatランダム
        float NextFloatUnit() { return NextFloatRange(0.0f, 1.0f); }

    private:
        std::random_device seedGenerator_;
        std::mt19937 engine_;
    };

}