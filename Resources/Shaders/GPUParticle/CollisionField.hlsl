#include "GPUParticle.hlsli"

StructuredBuffer<FieldForGPU> origalField : register(t0);
StructuredBuffer<uint> fieldIndexBuffer : register(t1);
RWStructuredBuffer<Particle> particle : register(u0);


struct Random
{
    uint32_t random;
};


ConstantBuffer<Random> gRandom : register(b0);

void AttractionField(FieldForGPU field,inout Particle particle){
    float32_t3 direction;
    if(field.fieldArea.type==0){
        direction = field.fieldArea.position - particle.translate.translate;
    }else if(field.fieldArea.type==1){
        direction = field.fieldArea.position - particle.translate.translate;
    }else if(field.fieldArea.type==2){
        float32_t3 position=closestPointOnSegment(
            particle.translate.translate,
            field.fieldArea.capsule.segment.origin+field.fieldArea.position,
            field.fieldArea.capsule.segment.diff+field.fieldArea.position);
        direction =  position - particle.translate.translate;
    }
    particle.velocity += normalize(direction) * field.field.attraction.attraction;
}

void ExternalForceField(FieldForGPU field,inout Particle particle,inout uint32_t seed){
    particle.velocity += randomRange(field.field.externalForce.externalForce.min,field.field.externalForce.externalForce.max,seed);
}

void UpdateField(FieldForGPU field,inout Particle particle,inout uint32_t seed){
    if(field.field.type==0){
        AttractionField(field,particle);
    }else if(field.field.type==1){
        ExternalForceField(field,particle,seed);
    }
}

[numthreads(threadBlockSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID , uint3 GTid : SV_GroupThreadID)
{
    uint32_t particleIndex = DTid.x;
    uint32_t fieldIndex = fieldIndexBuffer[GTid.y];
    uint32_t seed=gRandom.random;
    // 生きてるか
    if(origalField[fieldIndex].isAlive&&particle[particleIndex].isAlive){
        if((particle[particleIndex].collisionInfo.attribute & origalField[fieldIndex].collisionInfo.mask)!=0){
            // AABB
            if(origalField[fieldIndex].fieldArea.type==0){
                if(sdAABB(
                    particle[particleIndex].translate.translate,
                    origalField[fieldIndex].fieldArea.position,
                    origalField[fieldIndex].fieldArea.aabb.range.min+origalField[fieldIndex].fieldArea.position,
                    origalField[fieldIndex].fieldArea.aabb.range.max+origalField[fieldIndex].fieldArea.position) <= 0){
                    UpdateField(origalField[fieldIndex],particle[particleIndex],seed);
                }
            }
            // Sphere
            else if(origalField[fieldIndex].fieldArea.type==1){
                if(sdSphere(
                    particle[particleIndex].translate.translate,
                    origalField[fieldIndex].fieldArea.position,
                    origalField[fieldIndex].fieldArea.sphere.radius) <= 0){
                    UpdateField(origalField[fieldIndex],particle[particleIndex],seed);
                }
            }
            // Capsule
            else if(origalField[fieldIndex].fieldArea.type==2){
                if(sdCapsule(
                    particle[particleIndex].translate.translate,
                origalField[fieldIndex].fieldArea.capsule.segment.origin+origalField[fieldIndex].fieldArea.position,
                origalField[fieldIndex].fieldArea.capsule.segment.diff+origalField[fieldIndex].fieldArea.position,
                origalField[fieldIndex].fieldArea.capsule.radius) <= 0){
                    UpdateField(origalField[fieldIndex],particle[particleIndex],seed);
                }
            }
        }
    }
}