#include "GPUParticleShaderStructs.h"

AppendStructuredBuffer<GPUParticleShaderStructs::TrailsVertexData> trailsVertexData : register(u0);
StructuredBuffer<GPUParticleShaderStructs::TrailsData> trailsData : register(t0);
StructuredBuffer<GPUParticleShaderStructs::TrailsPosition> trailsPosition : register(t1);

[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int32_t trailsIndex = DTid.x;
    GPUParticleShaderStructs::TrailsData data = trailsData[trailsIndex];
    if (data.isAlive) {
        GPUParticleShaderStructs::TrailsPosition posData0, posData1;
        GPUParticleShaderStructs::TrailsVertexData vertexData;

        for (uint32_t i = data.startIndex; i < data.currentIndex - 1; i++) {
            posData0 = trailsPosition[i];
            posData1 = trailsPosition[i + 1];

            vertexData.vertex[0] = posData0.vertex[0];
            vertexData.vertex[1] = posData0.vertex[1];
            vertexData.vertex[2] = posData1.vertex[0];

            trailsVertexData.Append(vertexData);

            vertexData.vertex[0] = posData0.vertex[1];
            vertexData.vertex[1] = posData1.vertex[0];
            vertexData.vertex[2] = posData1.vertex[1];

            trailsVertexData.Append(vertexData);
        }
    }
}
