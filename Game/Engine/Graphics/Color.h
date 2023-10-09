#pragma once

#include "../Math/Vector4.h"
#include "../Math/Vector3.h"

class Color {
public:
    static Color HSVA(float h, float s, float v, float a = 1.0f);
    static Color RGBA(float r, float g, float b, float a = 1.0f);
    static uint32_t Merge(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    static uint32_t Convert(const Vector4& rgba);
    static Vector4 Convert(uint32_t rgba);

    Color() : rgba_(Vector4(1.0f,1.0f,1.0f,1.0f)) {}
    Color(const Color& color) :rgba_(color.rgba_){}
    Color(float r, float g, float b, float a = 1.0f) : rgba_(Vector4(r, g, b, a)) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, uint8_t bitDepth = 8) : rgba_(Convert(Merge(r, g, b, a))) {}
    explicit Color(uint32_t rgba) : rgba_(Convert(rgba)) {}
    explicit Color(const Vector4& rgba) : rgba_(rgba) {}

    operator const Vector4& () noexcept { return rgba_; }
    operator uint32_t() noexcept { return Convert(rgba_); }

    bool operator==(const Color& other) { return rgba_ == other.rgba_; };
    bool operator!=(const Color& other) { return rgba_ != other.rgba_; };

    void SetR(float r) { rgba_.x = r; }
    void SetG(float g) { rgba_.y = g; }
    void SetB(float b) { rgba_.z = b; }
    void SetA(float a) { rgba_.w = a; }

    void SetR(uint8_t r);
    void SetG(uint8_t g);
    void SetB(uint8_t b);
    void SetA(uint8_t a);

    float GetR() const { return rgba_.x; }
    float GetG() const { return rgba_.y; }
    float GetB() const { return rgba_.z; }
    float GetA() const { return rgba_.w; }

private:
    Vector4 rgba_;
};