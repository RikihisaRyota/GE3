#include "GPUParticleShaderStructs.h"

AppendStructuredBuffer<int> trailsStock : register(u0);
AppendStructuredBuffer<int> trailsCounter : register(u1);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(u2);
RWStructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(u3);
StructuredBuffer<GPUParticleShaderStructs::Particle> particles : register(t0);
[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int32_t trailsIndex = DTid.x;
    GPUParticleShaderStructs::TrailsData data = trailsData[trailsIndex];
    if(data.isAlive && data.trailsIndex == trailsIndex){
        GPUParticleShaderStructs::Particle particle = particles[data.particleIndex];
        // パーティクルが生きているか
        if(particle.isAlive){
            if(data.time >= data.interval) {        
                trailsPosition[data.currentIndex].position = particle.matWorld[3].xyz;
                
                //trailsPosition[data.currentIndex].texcoord = float32_t2(0.0f,0.0f);
                data.currentIndex++;
                if(data.currentIndex >= data.endIndex){
                    data.currentIndex = data.startIndex;
                }
                data.time = 0;
            } else {
                data.time++;
            }
            // 生きているindexを格納
            trailsCounter.Append(data.currentIndex);
        }else{
                // 死んでいたらindexを返し初期化
                trailsStock.Append(data.trailsIndex);
                data.trailsIndex = -1;
                data.isAlive = 0;
                for(uint32_t i = data.startIndex; i < data.endIndex;i++){
                    trailsPosition[i].position = float32_t3(0.0f,0.0f,0.0f);
                    //trailsPosition[i].texcoord = float32_t2(0.0f,0.0f);
                }
        }
        trailsData[trailsIndex] = data;
    }
}