struct VSInput {
    uint instanceID : SV_InstanceID;  // 各トレイルのインスタンスID
    uint vertexID : SV_VertexID;      // 各頂点のインデックス
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;

    // トレイルのデータを取得
    GPUParticleShaderStructs::TrailsData trail = trailsData[input.instanceID];
    if (trail.isAlive == 0) discard;

    // セグメント数を計算
    uint numSegments = (trail.currentIndex + GPUParticleShaderStructs::TrailsRange - trail.startIndex) % GPUParticleShaderStructs::TrailsRange;

    // `vertexID`に対応するトレイルの位置インデックスをラップ処理
    uint trailIndex = (trail.startIndex + input.vertexID) % TrailsRange;
    float3 position = trailsPosition[trailIndex].position;

    // ワールド行列を適用
    output.position = mul(float4(position, 1.0), WorldViewProjectionMatrix);

    // 進行度に基づいてテクスチャ座標を設定
    output.texCoord = float2(input.vertexID / (float)numSegments, 0.0);

    return output;
}
