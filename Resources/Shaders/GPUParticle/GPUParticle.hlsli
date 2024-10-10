
#define threadBlockSize 1024
#define meshThreadBlockSize  1024
#define emitterSize 1024
#define fieldSize 1024
#define processNum 1
#define maxParticleNum 2<<22

#define PI 3.14159265359f

/////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////
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

float4x4 MakeRotationMatrix(float32_t4 quaternion)
{
    float32_t x = quaternion.x;
    float32_t y = quaternion.y;
    float32_t z = quaternion.z;
    float32_t w = quaternion.w;

    float32_t xx = x * x;
    float32_t yy = y * y;
    float32_t zz = z * z;
    float32_t xy = x * y;
    float32_t xz = x * z;
    float32_t yz = y * z;
    float32_t wx = w * x;
    float32_t wy = w * y;
    float32_t wz = w * z;

    float32_t4x4 rotationMatrix;

    rotationMatrix[0][0] = 1.0f - 2.0f * (yy + zz);
    rotationMatrix[0][1] = 2.0f * (xy - wz);
    rotationMatrix[0][2] = 2.0f * (xz + wy);
    rotationMatrix[0][3] = 0.0f;

    rotationMatrix[1][0] = 2.0f * (xy + wz);
    rotationMatrix[1][1] = 1.0f - 2.0f * (xx + zz);
    rotationMatrix[1][2] = 2.0f * (yz - wx);
    rotationMatrix[1][3] = 0.0f;

    rotationMatrix[2][0] = 2.0f * (xz - wy);
    rotationMatrix[2][1] = 2.0f * (yz + wx);
    rotationMatrix[2][2] = 1.0f - 2.0f * (xx + yy);
    rotationMatrix[2][3] = 0.0f;

    rotationMatrix[3][0] = 0.0f;
    rotationMatrix[3][1] = 0.0f;
    rotationMatrix[3][2] = 0.0f;
    rotationMatrix[3][3] = 1.0f;

    return rotationMatrix;
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
    float32_t4x4 scaleMatrix=MakeScaleMatrix(scale);
    float32_t4x4 rotateMatrix=MakeRotationMatrix(rotate);
    float32_t4x4 translateMatrix=MakeTranslationMatrix(translate);

    return mul(mul(scaleMatrix,rotateMatrix),translateMatrix);
}

float32_t4x4 MakeAffine(float32_t scale, float32_t3 rotate, float32_t3 translate)
{
    float32_t4x4 scaleMatrix=MakeScaleMatrix(scale);
    float32_t4x4 rotateMatrix=MakeRotationMatrix(rotate);
    float32_t4x4 translateMatrix=MakeTranslationMatrix(translate);

    return mul(mul(scaleMatrix,rotateMatrix),translateMatrix);
}

float32_t4x4 MakeAffine(float32_t3 scale, float32_t4 rotate, float32_t3 translate)
{
    float32_t4x4 scaleMatrix=MakeScaleMatrix(scale);
    float32_t4x4 rotateMatrix=MakeRotationMatrix(rotate);
    float32_t4x4 translateMatrix=MakeTranslationMatrix(translate);

    return mul(mul(scaleMatrix,rotateMatrix),translateMatrix);
}

/////////////////////////////////////////////////////////////////////////////////////////
float32_t sdAABB(float32_t3 p, float32_t3 a, float32_t3 b) 
{
    float32_t3 q = max(a - p, p - b);
    return length(max(q, 0.0f));
}

float sdAABB(float3 p, float3 c, float3 a, float3 b)
{
    float3 halfExtents = (b - a) * 0.5f;
    float3 minBound = c - halfExtents;
    float3 maxBound = c + halfExtents;
    float3 q = max(max(minBound - p, 0.0f), p - maxBound);
    return length(q);
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
/////////////////////////////////////////////////////////////////////////////////////////
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
    uint32_t isEmitterLife;
    uint32_t pad;
};

struct ParticleAttributes {
	uint32_t attribute;
	uint32_t mask;
	float32_t2 pad;
};
struct Translate {
	Float3MinMax easing;
	float32_t3 translate;
	uint32_t isEasing;

    float32_t radius;
	float32_t attraction;
	float32_t2 pad;
};

struct ParticleParent{
    uint32_t isParent;
    uint32_t emitterType;
    uint32_t emitterCount;
    uint32_t pad;
};

struct TriangleInfo {
	float32_t3 vertex;
    //float32_t pad;
	float32_t3 weight;
    //float32_t pad1;
};

struct Particle
{
    TriangleInfo triangleInfo;

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
    
    Translate translate;

    float32_t4x4 worldMatrix;

    ParticleAttributes collisionInfo;

    ParticleParent parent;

    float32_t3 velocity;
    
    float32_t pad;
};

struct EmitterAABB
{
    Float3MinMax range;
};

struct EmitterSphere
{
    float32_t radius;
    float32_t3 pad;
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
    float32_t3 position;
    uint32_t type;
};

struct ScaleAnimation
{
    Float3StartEnd range;
    uint32_t isUniformScale;
    uint32_t isStaticSize;
    float32_t2 pad;
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
    uint32_t isStaticColor;
    float32_t3 pad;
};

struct EmitterFrequency
{
	int32_t interval;
	int32_t isLoop;
	int32_t emitterLife;
	int32_t isOnce;
};

struct EmitterTime {
	int32_t particleTime;
	int32_t emitterTime;
	uint32_t2 pad;
};

struct ParticleLifeSpan
{
    UintMinMax range;
    uint32_t isEmitterLife;
    float32_t3 pad;
};

struct EmitterParent {
	float32_t4x4 worldMatrix;
	uint32_t isParent;
	uint32_t emitterType;
	float32_t2 pad;
};

struct EmitterModel {
	uint32_t vertexBufferIndex;
	uint32_t indexBufferIndex;
    uint32_t vertexCount;
    uint32_t indexCount;
};

struct EmitterLocalTransform {
	float32_t3 scale;
	float32_t pad;
	float32_t3 translate;
	float32_t pad1;
    float32_t4 rotate;
};

struct TransformModelEmitter {
	Translate translate;

	ScaleAnimation scale;

	RotateAnimation rotate;

	Velocity3D velocity;

	EmitterColor color;

    EmitterFrequency frequency;

    EmitterTime time;

	ParticleLifeSpan particleLifeSpan;

	ParticleAttributes collisionInfo;

    EmitterParent parent;

    EmitterModel startModel;

    float32_t4x4 startModelWorldMatrix;

	EmitterModel endModel;

    float32_t4x4 endModelWorldMatrix;

	uint32_t textureIndex;

    int32_t emitterCount;

    uint32_t isAlive;

	float32_t pad;
};

struct TransformAreaEmitter {

		EmitterArea area;

		Translate translate;

		ScaleAnimation scale;

		RotateAnimation rotate;

		Velocity3D velocity;

		EmitterColor color;

		EmitterFrequency frequency;

		EmitterTime time;

		ParticleLifeSpan particleLifeSpan;

		ParticleAttributes collisionInfo;

		EmitterParent parent;

		EmitterModel model;

        float32_t4x4 modelWorldMatrix;

		uint32_t textureIndex;

		int32_t emitterCount;

		uint32_t isAlive;

		float32_t pad;
	};

struct MeshEmitter {
    Translate translate;

    EmitterLocalTransform localTransform;

	ScaleAnimation scale;

	RotateAnimation rotateAnimation;

	Velocity3D velocity3D;

	EmitterColor color;

    EmitterFrequency frequency;
    
    EmitterTime time;

	ParticleLifeSpan particleLifeSpan;

    ParticleAttributes collisionInfo;

    EmitterParent parent;

    EmitterModel model;

	uint32_t textureIndex;

    uint32_t numCreate;

    uint32_t isAlive;

    int32_t emitterCount;
};

struct VertexEmitter {
    Translate translate;

	EmitterLocalTransform localTransform;

	ScaleAnimation scale;

	RotateAnimation rotateAnimation;

	Velocity3D velocity3D;

	EmitterColor color;

    EmitterFrequency frequency;

    EmitterTime time;

	ParticleLifeSpan particleLifeSpan;

    ParticleAttributes collisionInfo;

    EmitterParent parent;

    EmitterModel model;

	uint32_t textureIndex;

    uint32_t isAlive;

    int32_t emitterCount;

    float32_t pad;
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

    EmitterParent parent;
};

struct CreateParticleNum
{
    uint32_t emitterNum;
    int32_t createParticleNum;
    uint32_t maxCreateParticleNum;
    uint32_t emitterType;
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
	Float3MinMax externalForce;
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

void ParticleArea(inout Particle particle,EmitterArea area ,inout uint32_t seed){
    particle.translate.translate = area.position;
    if(area.type==0){
        particle.translate.translate += randomRange(area.aabb.range.min, area.aabb.range.max, seed);
    }else if(area.type==1){
        float32_t3 normal,direction;
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction=normalize(normal);
        float32_t distanceFactor = randomRange(area.sphere.distanceFactor.min, area.sphere.distanceFactor.max, seed);
        particle.translate.translate +=  direction * (area.sphere.radius * distanceFactor);
    } else if(area.type==2){
        float32_t3 normal,direction,p;
        p = randomRangeSame(area.capsule.segment.origin+area.position, area.capsule.segment.diff+area.position,seed);
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction = normalize(normal);
        float32_t distanceFactor = randomRange(area.capsule.distanceFactor.min, area.capsule.distanceFactor.max, seed);
        direction *=  area.capsule.radius * distanceFactor;
        particle.translate.translate =  pointOnCapsule(p + direction, area.capsule.segment.origin +area.position,area.capsule.segment.diff +area.position,area.capsule.radius ,randomRange(area.capsule.distanceFactor.min,area.capsule.distanceFactor.max,seed));
    }
}

void ParticleScale(inout Particle particle,ScaleAnimation scale ,inout uint32_t seed){
    if(!scale.isUniformScale && !scale.isStaticSize){
        particle.scaleRange.min.x = randomRange(scale.range.start.min.x, scale.range.start.max.x,seed);
        particle.scaleRange.min.y = randomRange(scale.range.start.min.y, scale.range.start.max.y,seed);
        particle.scaleRange.min.z = randomRange(scale.range.start.min.z, scale.range.start.max.z,seed);
        
        particle.scaleRange.max.x = randomRange(scale.range.end.min.x, scale.range.end.max.x,seed);
        particle.scaleRange.max.y = randomRange(scale.range.end.min.y, scale.range.end.max.y,seed);
        particle.scaleRange.max.z = randomRange(scale.range.end.min.z, scale.range.end.max.z,seed);
    }
    else if(scale.isUniformScale&&!scale.isStaticSize) {
        particle.scaleRange.min=randomRangeSame(scale.range.start.min, scale.range.start.max,seed);
        particle.scaleRange.max=randomRangeSame(scale.range.end.min, scale.range.end.max,seed);
    }else if(!scale.isUniformScale&&scale.isStaticSize) {
        particle.scaleRange.min.x = randomRange(scale.range.start.min.x, scale.range.start.max.x,seed);
        particle.scaleRange.min.y = randomRange(scale.range.start.min.y, scale.range.start.max.y,seed);
        particle.scaleRange.min.z = randomRange(scale.range.start.min.z, scale.range.start.max.z,seed);
        particle.scaleRange.max = particle.scaleRange.min;
    }else {
        particle.scaleRange.min=randomRangeSame(scale.range.start.min, scale.range.start.max,seed);
        particle.scaleRange.max=particle.scaleRange.min;
    }
    particle.scale = particle.scaleRange.min;
}

void ParticleRotate(inout Particle particle,RotateAnimation rotateAnimation ,inout uint32_t seed){
    particle.rotateVelocity =  randomRange(rotateAnimation.rotateSpeed.min,rotateAnimation.rotateSpeed.max,seed);
    particle.rotate =  randomRange(rotateAnimation.initializeAngle.min,rotateAnimation.initializeAngle.max,seed);
}

void ParticleVelocity(inout Particle particle,Velocity3D velocity3D,inout uint32_t seed){
    particle.velocity = randomRange(velocity3D.range.min, velocity3D.range.max, seed);
}

void ParticleColor(inout Particle particle,EmitterColor color ,inout uint32_t seed){
   if(!color.isStaticColor){
        particle.colorRange.min=randomRange(color.range.start.min, color.range.start.max, seed);
        particle.colorRange.max=randomRange(color.range.end.min, color.range.end.max, seed);
    }else{
        particle.colorRange.min=randomRange(color.range.start.min, color.range.start.max, seed);
        particle.colorRange.max=particle.colorRange.min;
    }
    particle.color = particle.colorRange.min;
}

void ParticleLifeTime(inout Particle particle,ParticleLifeSpan particleLifeSpan,inout uint32_t seed){
    particle.particleLifeTime.isEmitterLife = particleLifeSpan.isEmitterLife;
    particle.particleLifeTime.maxTime = randomRange(particleLifeSpan.range.min, particleLifeSpan.range.max, seed);
    particle.particleLifeTime.time = 0;
}

void SetParent(inout Particle particle,EmitterParent parent,uint32_t emitterCount){
   particle.parent.isParent=parent.isParent;
   particle.parent.emitterType=parent.emitterType;
   particle.parent.emitterCount=emitterCount;
};

void ParticleReset(inout Particle particle){
    particle.isAlive = true;
    particle.isHit = false;
    particle.translate.isEasing = false;
    particle.parent.isParent = false;
    particle.particleLifeTime.isEmitterLife = false;
};

void ParticleTranslate(inout Particle particle,Translate translate,inout uint32_t seed){
    particle.translate = translate;
    particle.translate.translate=particle.translate.easing.min;
}

void ParticleTranslate(inout Particle particle,EmitterArea area ,Translate translate,inout uint32_t seed){
   particle.translate = translate;
   particle.translate.translate = area.position;
    if(area.type==0){
        particle.translate.translate += randomRange(area.aabb.range.min, area.aabb.range.max, seed);
    }else if(area.type==1){
        float32_t3 normal,direction;
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction=normalize(normal);
        float distanceFactor = randomRange(area.sphere.distanceFactor.min, area.sphere.distanceFactor.max, seed);
        direction *= area.sphere.radius * distanceFactor;
        particle.translate.translate += direction;
    } else if(area.type==2){
        float32_t3 normal,direction,p;
        p = randomRangeSame(area.capsule.segment.origin+area.position, area.capsule.segment.diff+area.position,seed);
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction = normalize(normal);
        float distanceFactor = randomRange(area.capsule.distanceFactor.min, area.capsule.distanceFactor.max, seed);
        direction *=  area.capsule.radius * distanceFactor;
        particle.translate.translate =  pointOnCapsule(p + direction, area.capsule.segment.origin +area.position,area.capsule.segment.diff +area.position,area.capsule.radius ,randomRange(area.capsule.distanceFactor.min,area.capsule.distanceFactor.max,seed));
    }
    particle.translate.easing.min = particle.translate.translate;
}

void ParticleTriangleInfo(inout Particle particle,TriangleInfo triangleInfo){
    particle.triangleInfo=triangleInfo;
}


void CreateParticle(inout Particle particle,Emitter emitter ,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);
    
    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotateAnimation,seed);
    
    ParticleArea(particle, emitter.area,seed);

    ParticleVelocity(particle, emitter.velocity3D,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout Particle particle,VertexEmitter emitter,float32_t3 translate,TriangleInfo triangleInfo,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);

    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotateAnimation,seed);

    ParticleTriangleInfo(particle,triangleInfo);

    ParticleTranslate(particle,emitter.translate,seed);
    particle.translate.translate = translate;
    
    ParticleVelocity(particle, emitter.velocity3D,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout Particle particle,MeshEmitter emitter,float32_t3 translate,TriangleInfo triangleInfo, inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;

    ParticleReset(particle);
    
    SetParent(particle, emitter.parent,emitterCount);

    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotateAnimation,seed);

    ParticleTriangleInfo(particle,triangleInfo);
    
    ParticleTranslate(particle,emitter.translate,seed);
    particle.translate.translate = translate;
    
    ParticleVelocity(particle, emitter.velocity3D,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout Particle particle,TransformModelEmitter emitter ,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);
    
    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);
    
    ParticleTranslate(particle,emitter.translate,seed);
    
    ParticleVelocity(particle, emitter.velocity,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout Particle particle,TransformAreaEmitter emitter ,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);

    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);
    
    ParticleTranslate(particle,emitter.area,emitter.translate,seed);
    
    ParticleVelocity(particle, emitter.velocity,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}


