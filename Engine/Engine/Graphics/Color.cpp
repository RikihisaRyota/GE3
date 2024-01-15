#include "Color.h"

#include <algorithm>

Color Color::HSVA(float h, float s, float v, float a) {
    float r = v, g = v, b = v;
    if (s > 0.0f) {
        h = std::fmod(h, 1.0f);
        h *= 6.0f;
        int32_t i = int32_t(h);
        float f = h - float(i);
        switch (i) {
        default:
        case 0:
            g *= 1.0f - s * (1.0f - f);
            b *= 1.0f - s;
            break;
        case 1:
            r *= 1.0f - s * f;
            b *= 1.0f - s;
            break;
        case 2:
            r *= 1.0f - s;
            b *= 1.0f - s * (1.0f - f);
            break;
        case 3:
            r *= 1.0f - s;
            g *= 1.0f - s * f;
            break;
        case 4:
            r *= 1.0f - s * (1.0f - f);
            g *= 1.0f - s;
            break;
        case 5:
            g *= 1.0f - s;
            b *= 1.0f - s * f;
            break;
        }
    }
    return { r, g, b, a };
}

Color Color::RGBA(float r, float g, float b, float a) {
    return { r, g, b, a };
}

uint32_t Color::Merge(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (r << 24u | g << 16u | b << 8u | a);
}

uint32_t Color::Convert(const Vector4& rgba) {
    uint8_t r = uint8_t(std::min(uint32_t(rgba.x * 0xFFu), 0xFFu));
    uint8_t g = uint8_t(std::min(uint32_t(rgba.y * 0xFFu), 0xFFu));
    uint8_t b = uint8_t(std::min(uint32_t(rgba.z * 0xFFu), 0xFFu));
    uint8_t a = uint8_t(std::min(uint32_t(rgba.w * 0xFFu), 0xFFu));
    return Merge(r, g, b, a);
}

Vector4 Color::Convert(uint32_t rgba) {
    constexpr float reci = 1.0f / 255.0f;
    auto GetElement = [&](uint32_t index) {
        return ((rgba & (0xFFu << index)) >> (8 * index)) * reci;
        };
    Vector4 result;
    result.x = std::clamp(GetElement(3), 0.0f, 1.0f);
    result.y = std::clamp(GetElement(2), 0.0f, 1.0f);
    result.z = std::clamp(GetElement(1), 0.0f, 1.0f);
    result.w = std::clamp(GetElement(0), 0.0f, 1.0f);
    return result;
}

void Color::SetR(uint8_t r) {
    constexpr float reci = 1.0f / 255.0f;
    rgba_.x = std::clamp(float(r) * reci, 0.0f, 1.0f);
}

void Color::SetG(uint8_t g) {
    constexpr float reci = 1.0f / 255.0f;
    rgba_.y = std::clamp(float(g) * reci, 0.0f, 1.0f);
}

void Color::SetB(uint8_t b) {
    constexpr float reci = 1.0f / 255.0f;
    rgba_.z = std::clamp(float(b) * reci, 0.0f, 1.0f);
}

void Color::SetA(uint8_t a) {
    constexpr float reci = 1.0f / 255.0f;
    rgba_.w = std::clamp(float(a) * reci, 0.0f, 1.0f);
}


