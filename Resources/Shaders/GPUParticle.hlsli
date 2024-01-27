
#define threadBlockSize 128

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct Particle
{
    float3 scale;
    float pad1;
    float3 velocity;
    float pad2;
    float3 rotate;
    float pad3;
    float3 translate;
    float pad4;
    uint isAlive;
    float3 pad5;
    uint isHit;
    float3 pad6;
    float aliveTime;
    float3 pad7;
};
struct Emitter
{
    uint createParticleNum;
    float3 min;
    uint maxParticleNum;
    float3 max;
    float3 position;
};
float hash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15u);
    return float(seed) / float(0x7FFFFFFF);
}

float random(float min, float max, float seed)
{
    float range = max - min;
    return min + hash(uint(seed)) * range;
}
uint RandomSeed(uint seed)
{
    seed = (seed * 1664525u + 1013904223u);
    return seed;
}

float Random(uint seed)
{
    return float(RandomSeed(seed)) / 4294967296.0f;
}

uint RandomIntRange(uint min, uint max,uint seed)
{
    return min + (RandomSeed(seed) % (max - min + 1));
}

float RandomRange(float min, float max, uint seed)
{
    seed = RandomSeed(seed);
    float normalized = float(seed) / 4294967296.0f;
    return min + normalized * (max - min);
}
matrix MakeScaleMatrix(float3 scale)
{
    return matrix(
    scale.x, 0.0f, 0.0f, 0.0f,
    0.0f, scale.y, 0.0f, 0.0f,
    0.0f, 0.0f, scale.z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);
}

matrix MakeScaleMatrix(float scale)
{
    return matrix(
    scale, 0.0f, 0.0f, 0.0f,
    0.0f, scale, 0.0f, 0.0f,
    0.0f, 0.0f, scale, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);
}

matrix MakeRotationMatrixX(float x)
{
    float cosine = cos(x);
    float sine = sin(x);
    return matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosine, sine, 0.0f,
        0.0f, -sine, cosine, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

matrix MakeRotationMatrixY(float y)
{
    float cosine = cos(y);
    float sine = sin(y);
    return matrix(
        cosine, 0.0f, -sine, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sine, 0.0f, cosine, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

matrix MakeRotationMatrixZ(float z)
{
    float cosine = cos(z);
    float sine = sin(z);
    return matrix(
        cosine, sine, 0.0f, 0.0f,
        -sine, cosine, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

matrix MakeRotationMatrix(float3 rotate)
{
    return mul(mul(MakeRotationMatrixX(rotate.x), MakeRotationMatrixY(rotate.y)), MakeRotationMatrixZ(rotate.z));

}

matrix MakeTranslationMatrix(float3 translate)
{
    return matrix(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    translate.x, translate.y, translate.z, 1.0f
);
}


matrix MakeAffine(float3 scale, float3 rotate, float3 translate)
{
    return matrix(mul(mul(MakeScaleMatrix(scale), MakeRotationMatrix(rotate)), MakeTranslationMatrix(translate)));

}

matrix MakeAffine(float scale, float3 rotate, float3 translate)
{
    return matrix(mul(mul(MakeScaleMatrix(scale), MakeRotationMatrix(rotate)), MakeTranslationMatrix(translate)));

}
float4x4 Inverse(float4x4 m)
{
    float4 c0 = m[0];
    float4 c1 = m[1];
    float4 c2 = m[2];
    float4 c3 = m[3];

    float4 r0 = float4(c1.y * c2.z * c3.w - c1.z * c2.y * c3.w + c1.z * c2.w * c3.y - c1.w * c2.z * c3.y - c1.y * c2.w * c3.z + c1.w * c2.y * c3.z,
        -c0.y * c2.z * c3.w + c0.z * c2.y * c3.w - c0.z * c2.w * c3.y + c0.w * c2.z * c3.y + c0.y * c2.w * c3.z - c0.w * c2.y * c3.z,
        c0.y * c1.z * c3.w - c0.z * c1.y * c3.w + c0.z * c1.w * c3.y - c0.w * c1.z * c3.y - c0.y * c1.w * c3.z + c0.w * c1.y * c3.z,
        -c0.y * c1.z * c2.w + c0.z * c1.y * c2.w - c0.z * c1.w * c2.y + c0.w * c1.z * c2.y + c0.y * c1.w * c2.z - c0.w * c1.y * c2.z);

    float4 r1 = float4(-c1.x * c2.z * c3.w + c1.z * c2.x * c3.w - c1.z * c2.w * c3.x + c1.w * c2.z * c3.x + c1.x * c2.w * c3.z - c1.w * c2.x * c3.z,
        c0.x * c2.z * c3.w - c0.z * c2.x * c3.w + c0.z * c2.w * c3.x - c0.w * c2.z * c3.x - c0.x * c2.w * c3.z + c0.w * c2.x * c3.z,
        -c0.x * c1.z * c3.w + c0.z * c1.x * c3.w - c0.z * c1.w * c3.x + c0.w * c1.z * c3.x + c0.x * c1.w * c3.z - c0.w * c1.x * c3.z,
        c0.x * c1.z * c2.w - c0.z * c1.x * c2.w + c0.z * c1.w * c2.x - c0.w * c1.z * c2.x - c0.x * c1.w * c2.z + c0.w * c1.x * c2.z);

    float4 r2 = float4(c1.x * c2.y * c3.w - c1.y * c2.x * c3.w + c1.y * c2.w * c3.x - c1.w * c2.y * c3.x - c1.x * c2.w * c3.y + c1.w * c2.x * c3.y,
        -c0.x * c2.y * c3.w + c0.y * c2.x * c3.w - c0.y * c2.w * c3.x + c0.w * c2.y * c3.x + c0.x * c2.w * c3.y - c0.w * c2.x * c3.y,
        c0.x * c1.y * c3.w - c0.y * c1.x * c3.w + c0.y * c1.w * c3.x - c0.w * c1.y * c3.x - c0.x * c1.w * c3.y + c0.w * c1.x * c3.y,
        -c0.x * c1.y * c2.w + c0.y * c1.x * c2.w - c0.y * c1.w * c2.x + c0.w * c1.y * c2.x + c0.x * c1.w * c2.y - c0.w * c1.x * c2.y);

    float4 r3 = float4(-c1.x * c2.y * c3.z + c1.y * c2.x * c3.z - c1.y * c2.z * c3.x + c1.z * c2.y * c3.x + c1.x * c2.z * c3.y - c1.z * c2.x * c3.y,
        c0.x * c2.y * c3.z - c0.y * c2.x * c3.z + c0.y * c2.z * c3.x - c0.z * c2.y * c3.x - c0.x * c2.z * c3.y + c0.z * c2.x * c3.y,
        -c0.x * c1.y * c3.z + c0.y * c1.x * c3.z - c0.y * c1.z * c3.x + c0.z * c1.y * c3.x + c0.x * c1.z * c3.y - c0.z * c1.x * c3.y,
        c0.x * c1.y * c2.z - c0.y * c1.x * c2.z + c0.y * c1.z * c2.x - c0.z * c1.y * c2.x - c0.x * c1.z * c2.y + c0.z * c1.x * c2.y);

    float determinant = dot(m[0], r0);
    if (abs(determinant) < 1e-6)
        return m;

    return float4x4(r0 / determinant, r1 / determinant, r2 / determinant, r3 / determinant);
}
