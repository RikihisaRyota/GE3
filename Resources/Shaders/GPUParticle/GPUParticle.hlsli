#include "GPUParticleShaderStructs.h"

#define PI 3.14159265359f

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR;

    uint32_t instanceId : SV_InstanceID;

};
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

float32_t4 randomRange(float32_t4 min, float32_t4 max, inout uint32_t seed) {
    float32_t4 result;
    result.x=lerp(min.x, max.x, randFloat(seed));
    result.y=lerp(min.y, max.y, randFloat(seed));
    result.z=lerp(min.z, max.z, randFloat(seed));
    result.w=lerp(min.w, max.w, randFloat(seed));
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
float32_t4x4 MakeAffine(float32_t3 scale, float32_t4x4 rotate, float32_t3 translate){
    float32_t4x4 mat={
        1.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        0.0f,0.0f,0.0f,1.0f
    };

    mat[0].x = scale.x * rotate[0].x;
    mat[0].y = scale.x * rotate[0].y;
    mat[0].z = scale.x * rotate[0].z;

    mat[1].x = scale.y * rotate[1].x;
    mat[1].y = scale.y * rotate[1].y;
    mat[1].z = scale.y * rotate[1].z;

    mat[2].x = scale.z * rotate[2].x;
    mat[2].y = scale.z * rotate[2].y;
    mat[2].z = scale.z * rotate[2].z;

    mat[3].x = translate.x;
    mat[3].y = translate.y;
    mat[3].z = translate.z;
    mat[3].w = 1.0f;

    return mat;
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

//int32_t GetFreeList(inout int32_t freeListIndex) {
//    int32_t index = -1;
//    InterlockedAdd(freeListIndex, -1, index);
//    if(index <= -1){
//        InterlockedAdd(freeListIndex, 1);
//    }
//    return index;
//}
//
//void ReturnFreeList(inout int32_t freeListIndex){
//    InterlockedAdd(freeListIndex, 1);
//}

/////////////////////////////////////////////////////////////////////////////////////////
void ParticleArea(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::EmitterArea area ,inout uint32_t seed){
    particle.translate.translate = area.position;
    if(area.type==0){
        particle.translate.translate += randomRange(area.aabb.area.min, area.aabb.area.max, seed);
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
        p = randomRangeSame(area.capsule.segment.origin, area.capsule.segment.diff,seed);
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction = normalize(normal);
        float32_t distanceFactor = randomRange(area.capsule.distanceFactor.min, area.capsule.distanceFactor.max, seed);
        direction *=  area.capsule.radius * distanceFactor;
        particle.translate.translate =  pointOnCapsule(p + direction, area.capsule.segment.origin ,area.capsule.segment.diff ,area.capsule.radius ,randomRange(area.capsule.distanceFactor.min,area.capsule.distanceFactor.max,seed));
    }
}

void ParticleScale(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::ScaleAnimation scale ,inout uint32_t seed){
    if(!scale.isUniformScale && !scale.isStaticSize){
        particle.scaleRange.min.x = randomRange(scale.range.start.min.x, scale.range.start.max.x,seed);
        particle.scaleRange.min.y = randomRange(scale.range.start.min.y, scale.range.start.max.y,seed);
        particle.scaleRange.min.z = randomRange(scale.range.start.min.z, scale.range.start.max.z,seed);
        
        particle.scaleRange.max.x = randomRange(scale.range.end.min.x, scale.range.end.max.x,seed);
        particle.scaleRange.max.y = randomRange(scale.range.end.min.y, scale.range.end.max.y,seed);
        particle.scaleRange.max.z = randomRange(scale.range.end.min.z, scale.range.end.max.z,seed);
        if(scale.isMedPoint){
            particle.medScale.x = randomRange(scale.mediumRange.min.x, scale.mediumRange.max.x,seed);
            particle.medScale.y = randomRange(scale.mediumRange.min.y, scale.mediumRange.max.y,seed);
            particle.medScale.z = randomRange(scale.mediumRange.min.z, scale.mediumRange.max.z,seed);
            particle.isMedPoint = true;
        }

    }
    else if(scale.isUniformScale&&!scale.isStaticSize) {
        particle.scaleRange.min=randomRangeSame(scale.range.start.min, scale.range.start.max,seed);
        particle.scaleRange.max=randomRangeSame(scale.range.end.min, scale.range.end.max,seed);
         if(scale.isMedPoint){
            particle.medScale = randomRangeSame(scale.mediumRange.min, scale.mediumRange.max,seed);
            particle.isMedPoint = true;
        }
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

void ParticleRotate(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::RotateAnimation rotateAnimation ,inout uint32_t seed){
    particle.rotateVelocity =  randomRange(rotateAnimation.rotateSpeed.min,rotateAnimation.rotateSpeed.max,seed);
    particle.rotate =  randomRange(rotateAnimation.initializeAngle.min,rotateAnimation.initializeAngle.max,seed);
}

void ParticleVelocity(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::Velocity3D velocity3D, GPUParticleShaderStructs::Acceleration3D acceleration,inout uint32_t seed){
    particle.acceleration = randomRange(acceleration.range.min, acceleration.range.max, seed);
    particle.velocity = randomRange(velocity3D.range.min, velocity3D.range.max, seed);
}

void ParticleColor(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::EmitterColor color ,inout uint32_t seed){
   if(!color.isStaticColor){
        particle.colorRange.min=randomRange(color.range.start.min, color.range.start.max, seed);
        particle.colorRange.max=randomRange(color.range.end.min, color.range.end.max, seed);
    }else{
        particle.colorRange.min=randomRange(color.range.start.min, color.range.start.max, seed);
        particle.colorRange.max=particle.colorRange.min;
    }
    particle.color = particle.colorRange.min;
}

void ParticleLifeTime(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::ParticleLifeSpan particleLifeSpan,inout uint32_t seed){
    particle.particleLifeTime.isEmitterLife = particleLifeSpan.isEmitterLife;
    particle.particleLifeTime.isCountDown = particleLifeSpan.isCountDown;
    particle.particleLifeTime.maxTime = randomRange(particleLifeSpan.range.min, particleLifeSpan.range.max, seed);
    particle.particleLifeTime.time = 0;
}

void SetParent(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::EmitterParent parent,uint32_t emitterCount){
   particle.parent.isParent=parent.isParent;
   particle.parent.emitterType=parent.emitterType;
   particle.parent.emitterCount=emitterCount;
};

void ParticleReset(inout GPUParticleShaderStructs::Particle particle){
    particle.isAlive = true;
    particle.isHit = false;
    particle.translate.isEasing = false;
    particle.parent.isParent = false;
    particle.particleLifeTime.isEmitterLife = false;
    particle.isMedPoint = false;
};

void ParticleTranslate(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::Translate translate,inout uint32_t seed){
    particle.translate = translate;
    particle.translate.translate=particle.translate.easing.min;
}

void ParticleTranslate(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::EmitterArea area ,GPUParticleShaderStructs::Translate translate,inout uint32_t seed){
   particle.translate = translate;
   particle.translate.translate = area.position;
    if(area.type==0){
        particle.translate.translate += randomRange(area.aabb.area.min, area.aabb.area.max, seed);
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

void ParticleTriangleInfo(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::TriangleInfo triangleInfo){
    particle.triangleInfo=triangleInfo;
}

//void ParticleTrail(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::ParticleTrails particleTrails,GPUParticleShaderStructs::TrailsPosition trailsPosition) {
//    if(particle.particleTrails.isTrails){
//        uint32_t startIndex=trailsPosition.startIndex;
//        if(trailsPosition.startIndex <= maxTrailNum){
//            trailsPosition.startIndex+=trailsRange;
//        }else{
//            trailsPosition.startIndex = 0;
//        }
//        particle.particleTrails.startIndex = startIndex;
//    }
//}

void CreateParticle(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::EmitterForGPU emitter,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);
    
    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);
    
    ParticleArea(particle, emitter.emitterArea,seed);

    ParticleVelocity(particle, emitter.velocity, emitter.acceleration,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::VertexEmitterForGPU emitter,float32_t3 translate,GPUParticleShaderStructs::TriangleInfo triangleInfo,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);

    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);

    ParticleTriangleInfo(particle,triangleInfo);

    ParticleTranslate(particle,emitter.translate,seed);
    particle.translate.translate = translate;
    
    ParticleVelocity(particle, emitter.velocity, emitter.acceleration,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::MeshEmitterForGPU emitter,float32_t3 translate,GPUParticleShaderStructs::TriangleInfo triangleInfo, inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;

    ParticleReset(particle);
    
    SetParent(particle, emitter.parent,emitterCount);

    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);

    ParticleTriangleInfo(particle,triangleInfo);
    
    ParticleTranslate(particle,emitter.translate,seed);
    particle.translate.translate = translate;
    
    ParticleVelocity(particle, emitter.velocity, emitter.acceleration,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}

void CreateParticle(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::TransformModelEmitterForGPU emitter ,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);
    
    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);
    
    ParticleTranslate(particle,emitter.translate,seed);
    
    ParticleVelocity(particle, emitter.velocity, emitter.acceleration,seed);
    
    ParticleColor(particle, emitter.color,seed); 


}

void CreateParticle(inout GPUParticleShaderStructs::Particle particle,GPUParticleShaderStructs::TransformAreaEmitterForGPU emitter ,inout uint32_t seed,uint32_t emitterCount){
    particle.textureIndex = emitter.textureIndex;
    particle.collisionInfo = emitter.collisionInfo;
    
    ParticleReset(particle);

    SetParent(particle, emitter.parent,emitterCount);
    
    ParticleLifeTime(particle, emitter.particleLifeSpan,seed);
    
    ParticleScale(particle, emitter.scale,seed);
    
    ParticleRotate(particle, emitter.rotate,seed);
    
    ParticleTranslate(particle,emitter.emitterArea,emitter.translate,seed);
    
    ParticleVelocity(particle, emitter.velocity, emitter.acceleration,seed);
    
    ParticleColor(particle, emitter.color,seed); 
}


