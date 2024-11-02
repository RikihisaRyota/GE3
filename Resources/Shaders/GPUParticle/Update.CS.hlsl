#include "GPUParticle.hlsli"
#include "GPUParticleShaderStructs.h"

RWStructuredBuffer<GPUParticleShaderStructs::Particle> input : register(u0);

AppendStructuredBuffer<uint> particleIndexCommands : register(u1);
AppendStructuredBuffer<uint> outputDrawIndexCommands : register(u2);

RWStructuredBuffer<GPUParticleShaderStructs::TrailsData> trails  : register(u3);

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
        if(input[index].isAlive)
        {
            float t = 0.0f; 
            switch (input[index].parent.emitterType) {
                case 0:
                {
                    GPUParticleShaderStructs::EmitterForGPU e = emitter[input[index].parent.emitterCount];
                    t = CheckParticleLifeSpan(input[index],e.particleLifeSpan,e.isAlive);
                }
                break;
                case 1:
                {
                    GPUParticleShaderStructs::VertexEmitterForGPU e = vertexEmitter[input[index].parent.emitterCount];
                    t = CheckParticleLifeSpan(input[index],e.particleLifeSpan,e.isAlive);
                }
                break;
                case 2:
                {
                    GPUParticleShaderStructs::MeshEmitterForGPU e = meshEmitter[input[index].parent.emitterCount];
                    t = CheckParticleLifeSpan(input[index],e.particleLifeSpan,e.isAlive);
                }
                break;
                case 3:
                {
                    GPUParticleShaderStructs::TransformModelEmitterForGPU e = transformModelEmitter[input[index].parent.emitterCount];
                    t = CheckParticleLifeSpan(input[index],e.particleLifeSpan,e.isAlive);
                }
                break;
                case 4:
                {
                    GPUParticleShaderStructs::TransformAreaEmitterForGPU e = transformAreaEmitter[input[index].parent.emitterCount];
                    t = CheckParticleLifeSpan(input[index],e.particleLifeSpan,e.isAlive);
                }
                break;
                default:
                break;
            }

            // 移動
            input[index].velocity+=input[index].acceleration;
            if(!input[index].translate.isEasing){
                input[index].translate.translate += input[index].velocity;
            }else{
                float32_t3  direction = input[index].translate.easing.max - input[index].translate.translate;
                float32_t radius=input[index].translate.radius;
                float32_t attraction=input[index].translate.attraction;
                // 速度があるとき
                if(length(direction)!=0.0f){
                    input[index].velocity += normalize(direction)*attraction;
                    // 速度が半径を超えないように
                    if(length(input[index].velocity) >= radius){
                        input[index].velocity= normalize(input[index].velocity)*radius;
                    }
                }

                if(length(input[index].translate.translate-input[index].translate.easing.max) <= radius /*|| input[index].particleLifeTime.time >= input[index].particleLifeTime.maxTime*/){
                    input[index].translate.translate=input[index].translate.easing.max;
                }else{
                    input[index].translate.translate += input[index].velocity;
                }
            }

            float32_t4x4 translateMatrix = MakeTranslationMatrix(input[index].translate.translate);

            // スケール
            input[index].scale = lerp(input[index].scaleRange.min, input[index].scaleRange.max, t);
            float32_t4x4 scaleMatrix = MakeScaleMatrix(input[index].scale);
            
            // 回転
            input[index].rotate += input[index].rotateVelocity;
            float32_t4x4 particleRotate = MakeRotationMatrixZ(input[index].rotate);
            
            // 親の移動を含めた最終的な位置を計算
            float32_t3 finalPosition;
            float32_t4x4 parentWorldMatrix;
            if(input[index].parent.isParent) {
                // 親のワールド行列を取得
                if(input[index].parent.emitterType==0){
                    parentWorldMatrix = emitter[input[index].parent.emitterCount].parent.worldMatrix;
                } else if(input[index].parent.emitterType==1){
                    parentWorldMatrix = vertexEmitter[input[index].parent.emitterCount].parent.worldMatrix;
                } else if(input[index].parent.emitterType==2){
                    parentWorldMatrix = meshEmitter[input[index].parent.emitterCount].parent.worldMatrix;
                }else if(input[index].parent.emitterType==3) {
                    parentWorldMatrix = transformModelEmitter[input[index].parent.emitterCount].parent.worldMatrix;
                }else if(input[index].parent.emitterType==4){
                    parentWorldMatrix = transformAreaEmitter[input[index].parent.emitterCount].parent.worldMatrix;
                }
                float32_t4 parentTranslation = mul(float32_t4(input[index].translate.translate, 1.0f),parentWorldMatrix);
                finalPosition = parentTranslation.xyz;
            } else {
                finalPosition = input[index].translate.translate;
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
            if(input[index].parent.isParent){
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
    
            input[index].color = lerp(input[index].colorRange.min, input[index].colorRange.max, t);
    
            if((t >= 1.0f))
            {
                input[index].isAlive = false;
                particleIndexCommands.Append(index);
            }
            else
            {
                if(input[index].parent.isParent){
                    
                    input[index].matWorld = mul(mul(mul(scaleMatrix, rotateMatrix), translateMatrix), parentWorldMatrix);
                }
                else{
                    input[index].matWorld = mul(mul(scaleMatrix, rotateMatrix), translateMatrix);
                }
                outputDrawIndexCommands.Append(index);
            }
            if(!input[index].particleLifeTime.isEmitterLife){
                input[index].particleLifeTime.time++;
            }
        }
    }
}
