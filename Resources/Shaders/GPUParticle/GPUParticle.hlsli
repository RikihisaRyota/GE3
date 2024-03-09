
#define threadBlockSize 1024
#define emitterSize 100

// Utility
struct UintMinMax
{
    uint min;
    uint max;
};


struct Float3MinMax
{
    float3 min;
    float3 max;
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
    uint instanceId : SV_InstanceID;
};

struct ParticleLifeTime
{
    uint time;
    uint maxTime;
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
    
    uint textureInidex;
    
    uint isAlive;
};

struct EmitterArea
{
    Float3MinMax range;
    float3 position;
};

struct ScaleAnimation
{
    Float3StartEnd range;
};

struct RotateAnimation
{
    float3 rotate;
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
    uint time;
    uint interval;
    uint isLoop;
    uint lifeTime;
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
    
    uint textureIndex;

    uint createParticleNum;
	
    uint isAlive;
};

struct CreateParticle
{
    uint emitterNum;
    int createParticleNum;
};
struct EmitterCounterBuffer
{
    uint emitterCounter;
    uint3 pad;
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