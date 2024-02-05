#pragma once

#include <cstdint>

#include "Engine/Math/Matrix4x4.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

struct ParticleForGPU {
    Matrix4x4 matWorld;
    Vector4 color;
};

struct ParticleScale {
    Vector3 startScale;
    Vector3 interimScale;
    Vector3 endScale;
    Vector3 currentScale;
};

struct ParticleRotate {
    Vector3 addRotate;
    Vector3 currentRotate;
};

struct ParticleColor {
    Vector4 startColor;
    Vector4 endColor;
    Vector4 currentColor;
};

struct ParticleAngel {
    float start;
    float end;
    float current;
};

struct ParticleAliveTime {
    int32_t maxTime;
    int32_t time;
    int32_t randomRange;
};

struct ParticleVelocity {
    float speed;
    Vector3 velocity;
    float randomRange;
};

struct ParticleSpawn {
    Vector3 position;
    float rangeX;
    float rangeY;
};

struct ParticleRandomScale {
    Vector3 startRandomRange;
    Vector3 interimRandomRange;
    Vector3 endRandomRange;
};

struct EmitterColor{
    Vector4 startColor_;
    Vector4 startBeginMinRandomColor_;
    Vector4 startBeginMaxRandomColor_;

    Vector4 endColor_;
    Vector4 endBeginMinRandomColor_;
    Vector4 endBeginMaxRandomColor_;
};

struct Emitter {
    ParticleSpawn spawn;
    ParticleAngel angle;
    ParticleRandomScale randomScale;
    ParticleScale scale;
    EmitterColor color;
    int32_t inOnce;
    int32_t flameInterval;
    int32_t aliveTime;
    bool isAlive;
};

struct ParticleMotion {
    ParticleScale scale;
    ParticleRotate rotate;
    Vector3 position;
    ParticleVelocity velocity;
    Vector3 acceleration_;
    ParticleColor  color;
    ParticleAliveTime aliveTime;
    bool isAlive;
    ParticleForGPU constantBuff;
};
