#include "GPUParticle.hlsli"

RWStructuredBuffer<Particle> Output : register(u0);

StructuredBuffer<Emitter> gEmitter : register(t0);

struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b0);

ConsumeStructuredBuffer<uint> particleIndexCommands : register(u1);

RWStructuredBuffer<CreateParticle> createParticle : register(u2);

struct CounterParticle
{
    int32_t count;
};


RWStructuredBuffer<CounterParticle> particleIndexCounter : register(u3);

void LifeTime(uint index, uint32_t emitterIndex,inout uint32_t seed)
{
    
    Output[index].particleLifeTime.maxTime = randomRange(gEmitter[emitterIndex].particleLifeSpan.range.min, gEmitter[emitterIndex].particleLifeSpan.range.max, seed);
    Output[index].particleLifeTime.time = 0;
}

void Scale(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].scaleRange.min.x = randomRange(gEmitter[emitterIndex].scale.range.start.min.x, gEmitter[emitterIndex].scale.range.start.max.x,seed);
    Output[index].scaleRange.min.y = randomRange(gEmitter[emitterIndex].scale.range.start.min.y, gEmitter[emitterIndex].scale.range.start.max.y,seed);
    Output[index].scaleRange.min.z = randomRange(gEmitter[emitterIndex].scale.range.start.min.z, gEmitter[emitterIndex].scale.range.start.max.z,seed);
    
    Output[index].scaleRange.max.x = randomRange(gEmitter[emitterIndex].scale.range.end.min.x, gEmitter[emitterIndex].scale.range.end.max.x,seed);
    Output[index].scaleRange.max.y = randomRange(gEmitter[emitterIndex].scale.range.end.min.y, gEmitter[emitterIndex].scale.range.end.max.y,seed);
    Output[index].scaleRange.max.z = randomRange(gEmitter[emitterIndex].scale.range.end.min.z, gEmitter[emitterIndex].scale.range.end.max.z,seed);
    
    Output[index].scale = Output[index].scaleRange.min;
}

void Rotate(uint index, uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].rotateVelocity = gEmitter[emitterIndex].rotateAnimation.rotate;
}

void Translate(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    if(gEmitter[emitterIndex].area.type==0){
        Output[index].translate = gEmitter[emitterIndex].area.position;
        Output[index].translate += randomRange(gEmitter[emitterIndex].area.aabb.range.min, gEmitter[emitterIndex].area.aabb.range.max, seed);
    }else if(gEmitter[emitterIndex].area.type==1){
        Output[index].translate = gEmitter[emitterIndex].area.position;
        float32_t3 normal,direction;
        normal.x = randomRange(-1.0f, 1.0f,seed);
        normal.y = randomRange(-1.0f, 1.0f,seed);
        normal.z = randomRange(-1.0f, 1.0f,seed);
        direction=normalize(normal);
        direction*= randomRange(0.0f, gEmitter[emitterIndex].area.sphere.radius, seed);
        Output[index].translate += direction;
    } else if(gEmitter[emitterIndex].area.type==2){
            float32_t3 normal,direction,p;
            p = randomRangeSame(gEmitter[emitterIndex].area.capsule.segment.origin, gEmitter[emitterIndex].area.capsule.segment.diff,seed);
            normal.x = randomRange(-1.0f, 1.0f,seed);
            normal.y = randomRange(-1.0f, 1.0f,seed);
            normal.z = randomRange(-1.0f, 1.0f,seed);
            direction=normalize(normal);
            direction*= randomRange(0.0f, gEmitter[emitterIndex].area.capsule.radius, seed);
            Output[index].translate =  pointOnCapsule(p + direction,gEmitter[emitterIndex].area.capsule.segment.origin,gEmitter[emitterIndex].area.capsule.segment.diff,gEmitter[emitterIndex].area.capsule.radius ,randomRange(-1.0f, 0.0f,seed));
    }
}

void Velocity(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].velocity = randomRange(gEmitter[emitterIndex].velocity3D.range.min, gEmitter[emitterIndex].velocity3D.range.max, seed);
}

void Color(uint index,  uint32_t emitterIndex,inout uint32_t seed)
{
    Output[index].colorRange.min=randomRange(gEmitter[emitterIndex].color.range.start.min, gEmitter[emitterIndex].color.range.start.max, seed);
    Output[index].colorRange.max=randomRange(gEmitter[emitterIndex].color.range.end.min, gEmitter[emitterIndex].color.range.end.max, seed);
    Output[index].color =Output[index].colorRange.min;
}

void Create(uint index,  uint32_t emitterIndex,uint32_t seed)
{
    Output[index].textureIndex = gEmitter[emitterIndex].textureIndex;
    Output[index].collisionInfo = gEmitter[emitterIndex].collisionInfo;
    
    Output[index].isAlive = 1;
    Output[index].isHit = 0;
    
    LifeTime(index, emitterIndex,seed);
    
    Scale(index, emitterIndex,seed);
    
    Rotate(index, emitterIndex,seed);
    
    Translate(index, emitterIndex,seed);
    
    Velocity(index, emitterIndex,seed);
    
    Color(index, emitterIndex,seed);   
}


[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    for (int i = 0; i < emitterSize; i++)
    {
        if(createParticle[i].createParticleNum > 0){
            int32_t createNum=-1; 
            InterlockedAdd(createParticle[i].createParticleNum, -1,createNum);
          
                if (createNum > 0)
                { 
                    int32_t counter=-1;
                    InterlockedAdd(particleIndexCounter[0].count, -1,counter);
                    if(counter>0){
                    int index = particleIndexCommands.Consume();
                    uint32_t seed = setSeed(index * gRandom.random);
                    uint32_t emitterIndex = createParticle[i].emitterNum;
                    Create(index, emitterIndex,seed);
                    break;
                    }
                }
        }
    }
}