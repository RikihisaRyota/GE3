float32_t3 Grayscale(float32_t3 color){
    float32_t value = dot(color,float32_t3(0.2125f,0.7154f,0.0721f));
    return float32_t3(value,value,value);
}

float32_t3 Sepia(float32_t3 color){
    float32_t3 value =Grayscale(color);
    return float32_t3(value.x*1.0f,value.y*74.0f/107.0f,value.z*43.0f/107.0f);
}

float32_t3 Vignette(float32_t3 color, float32_t2 texcoord) {
    float32_t2 correct = texcoord * (1.0f - texcoord);
    float vignette = correct.x * correct.y * 16.0f;
    vignette = saturate(pow(vignette, 0.8f));
    return color * vignette;
}

static const float32_t2 kIndex3x3[3][3]={
    {{-1.0f,-1.0f},{0.0f,-1.0f},{1.0f,-1.0f}},
    {{-1.0f,0.0f},{0.0f,0.0f},{1.0f,0.0f}},
    {{-1.0f,1.0f},{0.0f,1.0f},{1.0f,1.0f}},
};
static const float32_t kKernel3x3[3][3]={
    {{1.0f/9.0f},{1.0f/9.0f},{1.0f/9.0f}},
    {{1.0f/9.0f},{1.0f/9.0f},{1.0f/9.0f}},
    {{1.0f/9.0f},{1.0f/9.0f},{1.0f/9.0f}},
};

// float32_t3 Smoothing(uint32_t k, float32_t2 texcoord,Texture2D<float4> tex,SamplerState smp){
//     uint32_t width , height;
//     tex.GetDimensions(width ,height);
//     float32_t2 uvStepSize = float32_t2(rcp(width),rcp(height));
//     float32_t3 result={0.0f,0.0f,0.0f};

//     float32_t average = k * k;
//     for(uint32_t x=0;x<index;x++){
//         for(uint32_t y=0;y<index;y++){
//         kKernel3x3[x][y]=1.0f / average;
//         }
//     }
    
//     for(uint32_t x = -k;x < k;++x){
//         for(uint32_t y = -k;y < k;y++){
//             float32_t2 tmp = texcoord + kIndex3x3[x][y] * uvStepSize;
//             float32_t3 fetchColor = tex.Sample(smp,tmp).rgb;
//             result += fetchColor * kKernel3x3[x][y];
//         }
//     }
//     return result;
// }