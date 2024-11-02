#include "GPUParticleShaderStructs.h"

ConsumeStructuredBuffer<uint> trailsCounter : register(u0);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(u1);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(u2);
StructuredBuffer<GPUParticleShaderStructs::Particle> particles : register(t0);
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int32_t trailsIndex = DTid.x;
    GPUParticleShaderStructs::TrailsData data = trailsData[trailsIndex];
    GPUParticleShaderStructs::Particle particle = particles[data.particleIndex];
    // パーティクルが生きているか
    if(particle.isAlive){
        if(data.time >= data.interval) {        
            trailsPosition[data.currentIndex].position =  particle.matWorld[3].xyz;
            data.currentIndex++;
            data.time = 0;
        } else {
            data.time++;
        }
    }else{
            // 死んでいたらindexを返し初期化
            //trailsCounter.Append(data.trailsIndex);
            for(uint32_t i = data.startIndex; i < data.endIndex;i++){
                trailsPosition[i].position = float32_t3(0.0f,0.0f,0.0f);
            }
    }
    trailsData[trailsIndex] = data;
}