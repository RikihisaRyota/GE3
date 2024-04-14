
#define threadBlockSize 1024
#define emitterSize 100

// Utility
struct UintMinMax
{
    uint32_t min;
    uint32_t max;
    uint32_t2 pad;
};


struct Float3MinMax
{
    float3 min;
    uint32_t pad1;
    float3 max;
    uint32_t pad2;
};

struct Float4MinMax
{
    float4 min;
    float4 max;
};

struct Float3StartEnd
{
    Float3MinMax start;
    Float3MinMax end;
};

struct Float4StartEnd
{
    Float4MinMax start;
    Float4MinMax end;
};
//
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR;
    uint32_t instanceId : SV_InstanceID;
};

struct ParticleLifeTime
{
    uint32_t time;
    uint32_t maxTime;
};

struct Particle
{
    Float3MinMax scaleRange;
    float3 scale;

    float3 rotateVelocity;
    float3 rotate;
    
    float3 translate;
    float3 velocity;
    
    float4 color;
    
    ParticleLifeTime particleLifeTime;
    
    uint32_t textureInidex;
    
    uint32_t isAlive;
};

struct EmitterArea
{
    Float3MinMax range;
    float3 position;
    uint32_t pad;
};

struct ScaleAnimation
{
    Float3StartEnd range;
};

struct RotateAnimation
{
    float3 rotate;
    uint32_t pad;
};

struct Velocity3D
{
    Float3MinMax range;
};

struct EmitterColor
{
    Float4StartEnd range;
};

struct EmitterFrequency
{
    uint32_t time;
    uint32_t interval;
    uint32_t isLoop;
    uint32_t lifeTime;
};

struct ParticleLifeSpan
{
    UintMinMax range;
};

struct Emitter
{
    EmitterArea area;

    ScaleAnimation scale;

    RotateAnimation rotateAnimation;

    Velocity3D velocity3D;

    EmitterColor color;
	
    EmitterFrequency frequency;
    
    ParticleLifeSpan particleLifeSpan;
    
    uint32_t textureIndex;

    uint32_t createParticleNum;
	
    uint32_t isAlive;

    uint32_t pad;
};

struct CreateParticle
{
    uint32_t emitterNum;
    int32_t createParticleNum;
};
struct EmitterCounterBuffer
{
    uint32_t emitterCounter;
    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
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
    return min + (hash(uint(seed))) * 0.5f * range;
}

float randomRange(float2 uv, float seed)
{
    return frac(sin(dot(uv, float2(12.9898f, 78.233f)) + seed) * 43758.5453);

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