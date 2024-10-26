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

void RotateForceField(FieldForGPU field, inout Particle particle, inout uint32_t seed) {
    float32_t3 rotationAxis = normalize(field.field.rotateForce.direction); 
    float32_t angleRadians = field.field.rotateForce.rotateSpeed; 
    float32_t cosTheta = cos(angleRadians);
    float32_t sinTheta = sin(angleRadians);
    
    float32_t ux = rotationAxis.x, uy = rotationAxis.y, uz = rotationAxis.z;

    float32_t3x3 rotationMatrix;
    rotationMatrix[0][0] = cosTheta + ux * ux * (1 - cosTheta);
    rotationMatrix[0][1] = ux * uy * (1 - cosTheta) - uz * sinTheta;
    rotationMatrix[0][2] = ux * uz * (1 - cosTheta) + uy * sinTheta;

    rotationMatrix[1][0] = uy * ux * (1 - cosTheta) + uz * sinTheta;
    rotationMatrix[1][1] = cosTheta + uy * uy * (1 - cosTheta);
    rotationMatrix[1][2] = uy * uz * (1 - cosTheta) - ux * sinTheta;

    rotationMatrix[2][0] = uz * ux * (1 - cosTheta) - uy * sinTheta;
    rotationMatrix[2][1] = uz * uy * (1 - cosTheta) + ux * sinTheta;
    rotationMatrix[2][2] = cosTheta + uz * uz * (1 - cosTheta);

    float32_t3 rotatedVelocity;
    rotatedVelocity.x = dot(rotationMatrix[0], particle.velocity);
    rotatedVelocity.y = dot(rotationMatrix[1], particle.velocity);
    rotatedVelocity.z = dot(rotationMatrix[2], particle.velocity);

    particle.velocity = rotatedVelocity;
}


void UpdateField(FieldForGPU field,inout Particle particle,inout uint32_t seed){
    if(field.field.type==0){
        AttractionField(field,particle);
    }else if(field.field.type==1){
        ExternalForceField(field,particle,seed);
    }else if(field.field.type==2){
        RotateForceField(field,particle,seed);
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