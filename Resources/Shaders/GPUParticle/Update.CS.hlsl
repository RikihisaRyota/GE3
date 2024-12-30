#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

RWStructuredBuffer<GPUParticleShaderStructs::Particle> input : register(u0);
RWStructuredBuffer<uint32_t> freeListBuffer : register(u1);
RWStructuredBuffer<int32_t> freeListIndexBuffer : register(u2);

AppendStructuredBuffer<uint> appendAliveParticle : register(u3);

StructuredBuffer<GPUParticleShaderStructs::EmitterForGPU> emitter : register(t0);
StructuredBuffer<GPUParticleShaderStructs::VertexEmitterForGPU> vertexEmitter : register(t1);
StructuredBuffer<GPUParticleShaderStructs::MeshEmitterForGPU> meshEmitter : register(t2);
StructuredBuffer<GPUParticleShaderStructs::TransformModelEmitterForGPU> transformModelEmitter : register(t3);
StructuredBuffer<GPUParticleShaderStructs::TransformAreaEmitterForGPU> transformAreaEmitter : register(t4);

struct ViewProjection
{
    float32_t4x4 view; // ビュー変換行列
    float32_t4x4 projection; // プロジェクション変換行列
    float32_t4x4 inverseView;
    float32_t3 cameraPos; // カメラのワールド座標
    float32_t pad;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

float32_t CheckParticleLifeSpan(inout GPUParticleShaderStructs::Particle particle, GPUParticleShaderStructs::ParticleLifeSpan particleLifeSpan, bool emitterIsAlive) {
    // エミッター寿命を使わない場合
    if (!particle.particleLifeTime.isEmitterLife) {
        return float32_t(particle.particleLifeTime.time) / float32_t(particle.particleLifeTime.maxTime);
    }

    // エミッターが死んでいる場合の処理
    if (!emitterIsAlive) {
        // エミッターが死んでいてカウントダウンではない場合
        if (!particleLifeSpan.isCountDown) {
            return 1.0f; 
        } 
        // カウントダウン時、パーティクルの寿命に移行
        else {
            particle.particleLifeTime.isEmitterLife = false;
            return 0.0f;
        }
    }
    // エミッターが生きている場合
    return 0.0f; 
}


[numthreads(GPUParticleShaderStructs::ComputeThreadBlockSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    for(uint32_t i=0;i<GPUParticleShaderStructs::MaxProcessNum ;i++){
        uint index = DTid.x * GPUParticleShaderStructs::MaxProcessNum+ i;
        if(index > GPUParticleShaderStructs::MaxParticleNum){
            return;
        }
        if(index > GPUParticleShaderStructs::DivisionParticleNum){
            return;
        }
        GPUParticleShaderStructs::Particle inputParticle = input[index];
        if(inputParticle.isAlive)
        {
            float t = 0.0f; 
            switch (inputParticle.parent.emitterType) {
                case 0:
                {
                    GPUParticleShaderStructs::EmitterForGPU e = emitter[inputParticle.parent.emitterCount];
                    t = CheckParticleLifeSpan(inputParticle,e.particleLifeSpan,e.isAlive);
                }
                break;
                case 1:
                {
                    GPUParticleShaderStructs::VertexEmitterForGPU e = vertexEmitter[inputParticle.parent.emitterCount];
                    t = CheckParticleLifeSpan(inputParticle,e.particleLifeSpan,e.isAlive);
                }
                break;
                case 2:
                {
                    GPUParticleShaderStructs::MeshEmitterForGPU e = meshEmitter[inputParticle.parent.emitterCount];
                    t = CheckParticleLifeSpan(inputParticle,e.particleLifeSpan,e.isAlive);
                }
                break;
                case 3:
                {
                    GPUParticleShaderStructs::TransformModelEmitterForGPU e = transformModelEmitter[inputParticle.parent.emitterCount];
                    t = CheckParticleLifeSpan(inputParticle,e.particleLifeSpan,e.isAlive);
                }
                break;
                case 4:
                {
                    GPUParticleShaderStructs::TransformAreaEmitterForGPU e = transformAreaEmitter[inputParticle.parent.emitterCount];
                    t = CheckParticleLifeSpan(inputParticle,e.particleLifeSpan,e.isAlive);
                }
                break;
                default:
                break;
            }

            // 移動
            inputParticle.velocity+=inputParticle.acceleration;
            if(!inputParticle.translate.isEasing){
                inputParticle.translate.translate += inputParticle.velocity;
            }else{
                float32_t3  direction = inputParticle.translate.easing.max - inputParticle.translate.translate;
                float32_t radius=inputParticle.translate.radius;
                float32_t attraction=inputParticle.translate.attraction;
                // 速度があるとき
                if(length(direction)!=0.0f){
                    inputParticle.velocity += normalize(direction)*attraction;
                    // 速度が半径を超えないように
                    if(length(inputParticle.velocity) >= radius){
                        inputParticle.velocity= normalize(inputParticle.velocity)*radius;
                    }
                }

                if(length(inputParticle.translate.translate-inputParticle.translate.easing.max) <= radius /*|| inputParticle.particleLifeTime.time >= inputParticle.particleLifeTime.maxTime*/){
                    inputParticle.translate.translate=inputParticle.translate.easing.max;
                }else{
                    inputParticle.translate.translate += inputParticle.velocity;
                }
            }

            float32_t4x4 translateMatrix = MakeTranslationMatrix(inputParticle.translate.translate);

            // スケール
            if(inputParticle.isMedPoint && t <= 0.5f){
                inputParticle.scale = lerp(inputParticle.scaleRange.min, inputParticle.medScale, t);
            }else if(inputParticle.isMedPoint && t >= 0.5f){
                inputParticle.scale = lerp(inputParticle.medScale, inputParticle.scaleRange.max, t);
            }else{
                inputParticle.scale = lerp(inputParticle.scaleRange.min, inputParticle.scaleRange.max, t);
            }
            //float32_t4x4 scaleMatrix = MakeScaleMatrix(inputParticle.scale);
            
            // 回転
            inputParticle.rotate += inputParticle.rotateVelocity;
            float32_t4x4 particleRotate = MakeRotationMatrixZ(inputParticle.rotate);
            
            // 親の移動を含めた最終的な位置を計算
            float32_t3 finalPosition;
            float32_t4x4 parentWorldMatrix;
            if(inputParticle.parent.isParent) {
                // 親のワールド行列を取得
                if(inputParticle.parent.emitterType==0){
                    parentWorldMatrix = emitter[inputParticle.parent.emitterCount].parent.worldMatrix;
                } else if(inputParticle.parent.emitterType==1){
                    parentWorldMatrix = vertexEmitter[inputParticle.parent.emitterCount].parent.worldMatrix;
                } else if(inputParticle.parent.emitterType==2){
                    parentWorldMatrix = meshEmitter[inputParticle.parent.emitterCount].parent.worldMatrix;
                }else if(inputParticle.parent.emitterType==3) {
                    parentWorldMatrix = transformModelEmitter[inputParticle.parent.emitterCount].parent.worldMatrix;
                }else if(inputParticle.parent.emitterType==4){
                    parentWorldMatrix = transformAreaEmitter[inputParticle.parent.emitterCount].parent.worldMatrix;
                }
                float32_t4 parentTranslation = mul(float32_t4(inputParticle.translate.translate, 1.0f),parentWorldMatrix);
                finalPosition = parentTranslation.xyz;
            } else {
                finalPosition = inputParticle.translate.translate;
            }

            // カメラの位置と向きを取得する
            float32_t3 cameraPos = gViewProjection.cameraPos;
            float32_t3 cameraDir = normalize(finalPosition - cameraPos);
    
            // カメラの方向を基にビルボードの回転行列を計算する
            float32_t3 upVector = float32_t3(0.0f, 1.0f, 0.0f);
            float32_t3 sideVector = normalize(cross(upVector, cameraDir)); // カメラの方向と上方向の外積を取る
            float32_t3 newUpVector =normalize(cross(cameraDir, sideVector)) ; // カメラの方向と側方向の外積を取る
    
            float32_t4x4 billboardMatrix =
            {
                float32_t4(sideVector, 0.0f),
                float32_t4(newUpVector, 0.0f),
                float32_t4(cameraDir, 0.0f),
                float32_t4(0.0f, 0.0f, 0.0f, 1.0f)
            };

            float32_t4x4 finalRotateMatrix;
            if(inputParticle.parent.isParent){
                // 親の回転行列（移動を含まない）
                float32_t4x4 parentRotateMatrix = float32_t4x4(
                    parentWorldMatrix[0], parentWorldMatrix[1], parentWorldMatrix[2], float32_t4(0.0f, 0.0f, 0.0f, 1.0f)
                );
                finalRotateMatrix = mul(particleRotate, parentRotateMatrix);
            } else {
                finalRotateMatrix = particleRotate;
            }
            // パーティクルの回転を適用
            float32_t4x4 rotateMatrix = mul(finalRotateMatrix,billboardMatrix); 
    
            inputParticle.color = lerp(inputParticle.colorRange.min, inputParticle.colorRange.max, t);
    
            if((t >= 1.0f))
            {
                inputParticle.isAlive = false;
                // freeListにindexを返却
                int32_t freeListIndex = -1; 
                InterlockedAdd(freeListIndexBuffer[0], 1,freeListIndex);
                freeListBuffer[freeListIndex + 1] = index;
            }
            else
            {
                if(inputParticle.parent.isParent){
                    
                    inputParticle.matWorld = mul(MakeAffine(inputParticle.scale ,rotateMatrix ,inputParticle.translate.translate), parentWorldMatrix);
                }
                else{
                    inputParticle.matWorld =  MakeAffine(inputParticle.scale ,rotateMatrix,inputParticle.translate.translate);
                }
                appendAliveParticle.Append(index);
            }
            if(!inputParticle.particleLifeTime.isEmitterLife){
                inputParticle.particleLifeTime.time++;
            }
            input[index] = inputParticle;
        }
    }
}
