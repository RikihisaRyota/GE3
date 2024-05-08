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

// static const float32_t2 kIndex3x3[3][3]={
//     {{-1.0f,-1.0f},{0.0f,-1.0f},{1.0f,-1.0f}},
//     {{-1.0f,0.0f},{0.0f,0.0f},{1.0f,0.0f}},
//     {{-1.0f,1.0f},{0.0f,1.0f},{1.0f,1.0f}},
// };
// static const float32_t kKernel3x3[3][3]={
//     {{1.0f/9.0f},{1.0f/9.0f},{1.0f/9.0f}},
//     {{1.0f/9.0f},{1.0f/9.0f},{1.0f/9.0f}},
//     {{1.0f/9.0f},{1.0f/9.0f},{1.0f/9.0f}},
// };
static const float32_t PI=3.14159265f;
float Gauss(float32_t x,float32_t y,float32_t sigma){
    float32_t exponent=-(x*x+y*y)*rcp(2.0f*sigma*sigma);
    float32_t denominator=2.0f*PI*sigma*sigma;
    return exp(exponent)*rcp(denominator);
}

static const uint32_t kTexelSize = 5;
static const uint32_t kTexelCenter = kTexelSize / 2;
static const float32_t kTexelAverage = pow(float32_t(kTexelSize), 2);

float32_t3 Smoothing(float32_t2 originalTexcoord, Texture2D<float4> tex, SamplerState smp){
    uint32_t width, height;
    tex.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
    float32_t3 result = {0.0f, 0.0f, 0.0f};

    float32_t kernel = (1.0f / kTexelAverage);
    float32_t wight = 0.0f;
    for(uint32_t x = 0; x < kTexelSize; ++x){
        for(uint32_t y = 0; y < kTexelSize; ++y){
            wight+= Gauss(float32_t(int32_t(x) - int32_t(kTexelCenter)), float32_t(int32_t(y) - int32_t(kTexelCenter)),1.2f);
            float32_t2 texcoord = originalTexcoord + float32_t2(int32_t(x) - int32_t(kTexelCenter), int32_t(y) - int32_t(kTexelCenter)) * uvStepSize;
            if(texcoord.x < 0.0f || texcoord.y < 0.0f || texcoord.x > 1.0f || texcoord.y > 1.0f){
                continue;
            }
            float32_t3 fetchColor = tex.Sample(smp, texcoord).rgb;
            result += fetchColor * kernel;
        }
    }
    return result*rcp(wight);
}