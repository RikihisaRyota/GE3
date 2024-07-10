
#define threadBlockSize 1024
#define meshThreadBlockSize  1024
#define emitterSize 1024
#define fieldSize 1024
#define processNum 1
#define maxParticleNum 2<<22

#define PI 3.14159265359f

// Utility
struct UintMinMax
{
    uint32_t min;
    uint32_t max;
    uint32_t2 pad;
};

struct FloatMinMax
{
    float32_t min;
    float32_t max;
    float32_t2 pad;
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

struct ParticleAttributes {
	uint32_t mask;
	uint32_t attribute;
	float32_t2 pad;
};

struct Particle
{
    Float3MinMax scaleRange;

    ParticleLifeTime particleLifeTime;

    Float4MinMax colorRange;
    float32_t4 color;
    
    float32_t3 scale;
    uint32_t textureIndex;

    float32_t rotateVelocity;
    float32_t rotate;
    uint32_t isAlive;
    uint32_t isHit;
    
    float32_t3 translate;
    uint32_t pad2;

    float32_t3 velocity;
    uint32_t pad3;    

    float32_t4x4 worldMatrix;

    ParticleAttributes collisionInfo;
};

struct EmitterAABB
{
    Float3MinMax range;
    float32_t3 position;
    float pad;
};

struct EmitterSphere
{
    float32_t radius;
    float32_t3 position;
    FloatMinMax distanceFactor;
};
struct EmitterSegment {
		float32_t3 origin;
		float32_t pad;
		float32_t3 diff;
		float32_t pad1;
};
struct EmitterCapsule {
	EmitterSegment segment;
	FloatMinMax distanceFactor;
	float32_t radius;
	float32_t3 pad;
};
struct EmitterArea{
    EmitterAABB aabb;
    EmitterSphere sphere;
    EmitterCapsule capsule;
    uint32_t type;
    float32_t3 pad;
};

struct ScaleAnimation
{
    Float3StartEnd range;
    uint32_t isSame;
	float32_t3 pad;
};

struct RotateAnimation
{
    FloatMinMax initializeAngle;
    FloatMinMax rotateSpeed;
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
	uint32_t interval;
	uint32_t isLoop;
	uint32_t emitterLife;
	uint32_t pad;
};

struct EmitterTime {
		uint32_t particleTime;
		uint32_t emitterTime;
		uint32_t2 pad;
};

struct ParticleLifeSpan
{
    UintMinMax range;
};

struct MeshEmitter {
	ScaleAnimation scale;

	RotateAnimation rotateAnimation;

	Velocity3D velocity3D;

	EmitterColor color;

	ParticleLifeSpan particleLifeSpan;

    ParticleAttributes collisionInfo;

	uint32_t textureIndex;

    float32_t3 pad;
};

struct Emitter
{
    EmitterArea area;

    ScaleAnimation scale;

    RotateAnimation rotateAnimation;

    Velocity3D velocity3D;

    EmitterColor color;
	
    EmitterFrequency frequency;

    EmitterTime time;
    
    ParticleLifeSpan particleLifeSpan;
    
    uint32_t textureIndex;

    uint32_t createParticleNum;
	
    uint32_t isAlive;

    int32_t emitterCount;

    ParticleAttributes collisionInfo;
};

struct CreateParticleNum
{
    uint32_t emitterNum;
    int32_t createParticleNum;
};
struct EmitterCounterBuffer
{
    uint32_t emitterCounter;
    uint32_t3 pad;
};

struct Attraction {
	float attraction;
	float32_t3 pad;
};


struct ExternalForce {
	float32_t3 externalForce;
	float pad;
};

struct Field {
	Attraction attraction;
	ExternalForce externalForce;
	uint32_t type;
	float32_t3 pad;
};

struct FieldFrequency {
	uint32_t isLoop;
	uint32_t lifeTime;
	float32_t2 pad;
};

struct FieldForGPU {

    Field field;
	
    EmitterArea fieldArea;

	FieldFrequency frequency;

	ParticleAttributes collisionInfo;

	uint32_t isAlive;

	int32_t fieldCount;

    float32_t2 pad;
};

float32_t sdAABB(float32_t3 p, float32_t3 a, float32_t3 b) 
{
    float32_t3 q = max(a - p, p - b);
    return length(max(q, 0.0f));
}

float32_t sdAABB(float32_t3 p, float32_t3 c, float32_t3 a, float32_t3 b) 
{
    float32_t3 q = max(max(a - p, 0.0f), p - b);
    return length(q);
}


float32_t sdSphere(float32_t3 p, float32_t3 c, float32_t s) 
{
    return length(p - c) - s;
}

float32_t sdCapsule(float32_t3 p, float32_t3 a, float32_t3 b, float32_t r)
{
    float32_t3 pa = p - a, ba = b - a;
    float32_t h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float32_t3 closestPointOnSegment(float32_t3 p, float32_t3 a, float32_t3 b)
{
    float32_t3 pa = p - a;
    float32_t3 ba = b - a;
    float t = saturate(dot(pa, ba) / dot(ba, ba));
    return a + t * ba;
}

float32_t3 pointOnCapsule(float32_t3 p, float32_t3 a, float32_t3 b, float32_t r, float32_t targetDistance)
{
    float32_t3 closestPoint = closestPointOnSegment(p, a, b);

    float32_t3 direction = normalize(p - closestPoint);

    float32_t distance = targetDistance * r;

    float32_t3 pointOnSurface = closestPoint + direction * distance;

    return pointOnSurface;
}

float32_t sdTriangle(float32_t3 p, float32_t3 a, float32_t3 b, float32_t3 c) {
    float32_t3 ba = b - a;
    float32_t3 pa = p - a;
    float32_t3 cb = c - b;
    float32_t3 pb = p - b;
    float32_t3 ac = a - c;
    float32_t3 pc = p - c;
    float32_t3 nor = cross(ba, ac);

    return sqrt(
        (sign(dot(cross(ba, nor), pa)) +
         sign(dot(cross(cb, nor), pb)) +
         sign(dot(cross(ac, nor), pc)) < 2.0)
            ? min(min(
                  dot(ba * clamp(dot(ba, pa) / dot(ba, ba), 0.0, 1.0) - pa,
                      ba * clamp(dot(ba, pa) / dot(ba, ba), 0.0, 1.0) - pa),
                  dot(cb * clamp(dot(cb, pb) / dot(cb, cb), 0.0, 1.0) - pb,
                      cb * clamp(dot(cb, pb) / dot(cb, cb), 0.0, 1.0) - pb)),
              dot(ac * clamp(dot(ac, pc) / dot(ac, ac), 0.0, 1.0) - pc,
                  ac * clamp(dot(ac, pc) / dot(ac, ac), 0.0, 1.0) - pc))
            : dot(nor, pa) * dot(nor, pa) / dot(nor, nor));
}

float32_t3 closestPointOnTriangle(float32_t3 p, float32_t3 a, float32_t3 b, float32_t3 c) {
    float32_t3 ab = b - a;
    float32_t3 ac = c - a;
    float32_t3 ap = p - a;

    float32_t d1 = dot(ab, ap);
    float32_t d2 = dot(ac, ap);

    if (d1 <= 0.0 && d2 <= 0.0) return a;

    float32_t3 bp = p - b;
    float32_t d3 = dot(ab, bp);
    float32_t d4 = dot(ac, bp);

    if (d3 >= 0.0 && d4 <= d3) return b;

    float32_t vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
        return a + ab * (d1 / (d1 - d3));

    float32_t3 cp = p - c;
    float32_t d5 = dot(ab, cp);
    float32_t d6 = dot(ac, cp);

    if (d6 >= 0.0 && d5 <= d6) return c;

    float32_t vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0)
        return a + ac * (d2 / (d2 - d6));

    float32_t va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0)
        return b + (c - b) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));

    return a + ab * (d1 / (d1 + d3)) + ac * (d2 / (d2 + d6));
}

float32_t3 pointOnTriangle(float32_t3 p, float32_t3 a, float32_t3 b, float32_t3 c, float32_t targetDistance) {
    float32_t3 closestPoint = closestPointOnTriangle(p, a, b, c);
    float32_t3 direction = normalize(p - closestPoint);
    return closestPoint + direction * targetDistance;
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
float32_t2 randomRange(float32_t2 min, float32_t2 max, inout uint32_t seed) {
    float32_t2 result;
    result.x=lerp(min.x, max.x, randFloat(seed));
    result.y=lerp(min.y, max.y, randFloat(seed));
    return result;
}
float32_t3 randomRange(float32_t3 min, float32_t3 max, inout uint32_t seed) {
    float32_t3 result;
    result.x=lerp(min.x, max.x, randFloat(seed));
    result.y=lerp(min.y, max.y, randFloat(seed));
    result.z=lerp(min.z, max.z, randFloat(seed));
    return result;
}

float32_t3 randomRangeSame(float32_t3 min, float32_t3 max, inout uint32_t seed) {
    float32_t3 result;
    float32_t localSeed =randFloat(seed);
    result.x=lerp(min.x, max.x, localSeed);
    result.y=lerp(min.y, max.y, localSeed);
    result.z=lerp(min.z, max.z, localSeed);
    return result;
}

float32_t4 randomRange(float32_t4 min, float32_t4 max, inout uint32_t seed) {
    float32_t4 result;
    result.x=lerp(min.x, max.x, randFloat(seed));
    result.y=lerp(min.y, max.y, randFloat(seed));
    result.z=lerp(min.z, max.z, randFloat(seed));
    result.w=lerp(min.w, max.w, randFloat(seed));
    return result;
}

float32_t random(float32_t2 uv, float32_t seed)
{
    return frac(sin(dot(uv, float32_t2(12.9898f, 78.233f)) + seed) * 43758.5453);

}

float32_t4x4 MakeScaleMatrix(float32_t3  scale)
{
    return float32_t4x4(
    scale.x, 0.0f, 0.0f, 0.0f,
    0.0f, scale.y, 0.0f, 0.0f,
    0.0f, 0.0f, scale.z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);
}

float32_t4x4 MakeScaleMatrix(float32_t scale)
{
    return float32_t4x4(
    scale, 0.0f, 0.0f, 0.0f,
    0.0f, scale, 0.0f, 0.0f,
    0.0f, 0.0f, scale, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);
}

float32_t4x4 MakeRotationMatrixX(float32_t x)
{
    float cosine = cos(x);
    float sine = sin(x);
    return float32_t4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosine, sine, 0.0f,
        0.0f, -sine, cosine, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

float32_t4x4 MakeRotationMatrixY(float32_t y)
{
    float cosine = cos(y);
    float sine = sin(y);
    return float32_t4x4(
        cosine, 0.0f, -sine, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sine, 0.0f, cosine, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

float32_t4x4 MakeRotationMatrixZ(float32_t z)
{
    float cosine = cos(z);
    float sine = sin(z);
    return float32_t4x4(
        cosine, sine, 0.0f, 0.0f,
        -sine, cosine, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

float32_t4x4 MakeRotationMatrix(float32_t3  rotate)
{
    return mul(mul(MakeRotationMatrixX(rotate.x), MakeRotationMatrixY(rotate.y)), MakeRotationMatrixZ(rotate.z));

}

float32_t4x4 MakeTranslationMatrix(float32_t3  translate)
{
    return float32_t4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    translate.x, translate.y, translate.z, 1.0f
);
}


float32_t4x4 MakeAffine(float32_t3  scale, float32_t3  rotate, float32_t3  translate)
{
    return float32_t4x4(mul(mul(MakeScaleMatrix(scale), MakeRotationMatrix(rotate)), MakeTranslationMatrix(translate)));

}

float32_t4x4 MakeAffine(float32_t scale, float32_t3 rotate, float32_t3 translate)
{
    return float32_t4x4(mul(mul(MakeScaleMatrix(scale), MakeRotationMatrix(rotate)), MakeTranslationMatrix(translate)));

}

void ParticleLifeTime(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    particle.particleLifeTime.maxTime = randomRange(emitter.particleLifeSpan.range.min, emitter.particleLifeSpan.range.max, seed);
    particle.particleLifeTime.time = 0;
}

void ParticleScale(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    if(!emitter.scale.isSame){
        particle.scaleRange.min.x = randomRange(emitter.scale.range.start.min.x, emitter.scale.range.start.max.x,seed);
        particle.scaleRange.min.y = randomRange(emitter.scale.range.start.min.y, emitter.scale.range.start.max.y,seed);
        particle.scaleRange.min.z = randomRange(emitter.scale.range.start.min.z, emitter.scale.range.start.max.z,seed);
        
        particle.scaleRange.max.x = randomRange(emitter.scale.range.end.min.x, emitter.scale.range.end.max.x,seed);
        particle.scaleRange.max.y = randomRange(emitter.scale.range.end.min.y, emitter.scale.range.end.max.y,seed);
        particle.scaleRange.max.z = randomRange(emitter.scale.range.end.min.z, emitter.scale.range.end.max.z,seed);
    }else{
        particle.scaleRange.min=randomRangeSame(emitter.scale.range.start.min, emitter.scale.range.start.max,seed);
        particle.scaleRange.max=randomRangeSame(emitter.scale.range.end.min, emitter.scale.range.end.max,seed);
    }
    particle.scale = particle.scaleRange.min;
}

void ParticleRotate(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    particle.rotateVelocity =  randomRange(emitter.rotateAnimation.rotateSpeed.min,emitter.rotateAnimation.rotateSpeed.max,seed);
    particle.rotate =  randomRange(emitter.rotateAnimation.initializeAngle.min,emitter.rotateAnimation.initializeAngle.max,seed);
}

void ParticleTranslate(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    if(emitter.area.type==0){
        particle.translate = emitter.area.aabb.position;
        particle.translate += randomRange(emitter.area.aabb.range.min, emitter.area.aabb.range.max, seed);
    }else if(emitter.area.type==1){
        particle.translate = emitter.area.sphere.position;
        float32_t3 normal,direction;
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction=normalize(normal);
        float distanceFactor = randomRange(emitter.area.sphere.distanceFactor.min, emitter.area.sphere.distanceFactor.max, seed);
        direction *= emitter.area.sphere.radius * distanceFactor;
        particle.translate += direction;
    } else if(emitter.area.type==2){
        float32_t3 normal,direction,p;
        p = randomRangeSame(emitter.area.capsule.segment.origin, emitter.area.capsule.segment.diff,seed);
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction = normalize(normal);
        float distanceFactor = randomRange(emitter.area.capsule.distanceFactor.min, emitter.area.capsule.distanceFactor.max, seed);
        direction *=  emitter.area.capsule.radius * distanceFactor;
        particle.translate =  pointOnCapsule(p + direction, emitter.area.capsule.segment.origin ,emitter.area.capsule.segment.diff ,emitter.area.capsule.radius ,randomRange(emitter.area.capsule.distanceFactor.min,emitter.area.capsule.distanceFactor.max,seed));
    }
}

void ParticleVelocity(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    particle.velocity = randomRange(emitter.velocity3D.range.min, emitter.velocity3D.range.max, seed);
}

void ParticleColor(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    particle.colorRange.min=randomRange(emitter.color.range.start.min, emitter.color.range.start.max, seed);
    particle.colorRange.max=randomRange(emitter.color.range.end.min, emitter.color.range.end.max, seed);
    particle.color =particle.colorRange.min;
}

void CreateParticle(inout Particle particle,Emitter emitter ,inout uint32_t seed){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    particle.isAlive = 1;
    particle.isHit = 0;
    
    ParticleLifeTime(particle, emitter,seed);
    
    ParticleScale(particle, emitter,seed);
    
    ParticleRotate(particle, emitter,seed);
    
    ParticleTranslate(particle, emitter,seed);
    
    ParticleVelocity(particle, emitter,seed);
    
    ParticleColor(particle, emitter,seed); 
}

void ParticleLifeTime(inout Particle particle,MeshEmitter emitter ,inout uint32_t seed){
    particle.particleLifeTime.maxTime = randomRange(emitter.particleLifeSpan.range.min, emitter.particleLifeSpan.range.max, seed);
    particle.particleLifeTime.time = 0;
}

void ParticleScale(inout Particle particle,MeshEmitter emitter ,inout uint32_t seed){
    if(!emitter.scale.isSame){
        particle.scaleRange.min.x = randomRange(emitter.scale.range.start.min.x, emitter.scale.range.start.max.x,seed);
        particle.scaleRange.min.y = randomRange(emitter.scale.range.start.min.y, emitter.scale.range.start.max.y,seed);
        particle.scaleRange.min.z = randomRange(emitter.scale.range.start.min.z, emitter.scale.range.start.max.z,seed);
        
        particle.scaleRange.max.x = randomRange(emitter.scale.range.end.min.x, emitter.scale.range.end.max.x,seed);
        particle.scaleRange.max.y = randomRange(emitter.scale.range.end.min.y, emitter.scale.range.end.max.y,seed);
        particle.scaleRange.max.z = randomRange(emitter.scale.range.end.min.z, emitter.scale.range.end.max.z,seed);
    }else{
        particle.scaleRange.min=randomRangeSame(emitter.scale.range.start.min, emitter.scale.range.start.max,seed);
        particle.scaleRange.max=randomRangeSame(emitter.scale.range.end.min, emitter.scale.range.end.max,seed);
    }
    particle.scale = particle.scaleRange.min;
}

void ParticleRotate(inout Particle particle,MeshEmitter emitter ,inout uint32_t seed){
    particle.rotateVelocity =  randomRange(emitter.rotateAnimation.rotateSpeed.min,emitter.rotateAnimation.rotateSpeed.max,seed);
    particle.rotate =  randomRange(emitter.rotateAnimation.initializeAngle.min,emitter.rotateAnimation.initializeAngle.max,seed);
}

void ParticleVelocity(inout Particle particle,MeshEmitter emitter ,inout uint32_t seed){
    particle.velocity = randomRange(emitter.velocity3D.range.min, emitter.velocity3D.range.max, seed);
}

void ParticleColor(inout Particle particle,MeshEmitter emitter ,inout uint32_t seed){
    particle.colorRange.min=randomRange(emitter.color.range.start.min, emitter.color.range.start.max, seed);
    particle.colorRange.max=randomRange(emitter.color.range.end.min, emitter.color.range.end.max, seed);
    particle.color =particle.colorRange.min;
}

void CreateParticle(inout Particle particle,MeshEmitter emitter ,float32_t3 translate,inout uint32_t seed){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    particle.isAlive = 1;
    particle.isHit = 0;

    particle.translate = translate;
    
    ParticleLifeTime(particle, emitter,seed);
    
    ParticleScale(particle, emitter,seed);
    
    ParticleRotate(particle, emitter,seed);
    
    ParticleVelocity(particle, emitter,seed);
    
    ParticleColor(particle, emitter,seed); 
}