
#define threadBlockSize 8
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
    float32_t3 min;
    uint32_t pad1;
    float32_t3 max;
    uint32_t pad2;
};

struct Float4MinMax
{
    float32_t4 min;
    float32_t4 max;
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
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR;
    uint32_t instanceId : SV_InstanceID;
};

struct ParticleLifeTime
{
    uint32_t time;
    uint32_t maxTime;
    uint32_t2 pad;
};

struct Particle
{
    Float3MinMax scaleRange;

    ParticleLifeTime particleLifeTime;

    float32_t4 color;
    
    float32_t3 scale;
    uint32_t textureInidex;

    float32_t rotateVelocity;
    float32_t rotate;
    uint32_t isAlive;
    uint32_t pad1;
    
    float32_t3 translate;
    uint32_t pad2;

    float32_t3 velocity;
    uint32_t pad3;    
};

struct EmitterAABB
{
    Float3MinMax range;
};

struct EmitterSphere
{
    float32_t radius;
    float32_t3 pad;
};

struct EmitterArea{
    EmitterAABB aabb;
    EmitterSphere sphere;
    float32_t3 position;
    uint32_t type;
};

struct ScaleAnimation
{
    Float3StartEnd range;
};

struct RotateAnimation
{
    float32_t rotate;
    uint32_t3 pad;
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
    uint32_t3 pad;
};

float sdSphere(float32_t3 p, float32_t s )
{
  return length(p)-s;
}


uint32_t randInt(uint32_t seed) {
    seed ^= seed << 13;
    seed ^= seed >> 17;
	seed ^= seed << 5;
    return seed;
}

uint32_t setSeed(uint32_t seed) {
    seed = randInt(seed); 
    seed = randInt(seed); 
    seed = randInt(seed); 
    return seed ;
}

float32_t randFloat(inout uint32_t seed) {
    seed = setSeed(seed);
    return frac(float32_t(randInt(seed)) / float32_t(0xFFFFFFFFu));
}

float32_t  randomRange(float32_t min, float max,inout uint32_t  seed) {
    return lerp(min, max, randFloat(seed));
}
float32_t random(float32_t2 uv, float32_t seed)
{
    return frac(sin(dot(uv, float32_t2(12.9898f, 78.233f)) + seed) * 43758.5453);

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