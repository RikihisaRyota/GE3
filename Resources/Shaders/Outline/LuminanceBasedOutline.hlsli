static const float32_t kPrewittHorizontalKernel[3][3]={
    {-1.0f/6.0f,0.0f,1.0f/6.0f},
    {-1.0f/6.0f,0.0f,1.0f/6.0f},
    {-1.0f/6.0f,0.0f,1.0f/6.0f},
};

static const float32_t kPrewittVerticalKernel[3][3]={
    {-1.0f/6.0f,-1.0f/6.0f,-1.0f/6.0f},
    {0.0f,0.0f,0.0f},
    {1.0f/6.0f,1.0f/6.0f,1.0f/6.0f},
};

float32_t Luminance(float32_t3 v){
    return dot(v,float32_t3(0.2125f,0.7154f,0.0721f));
}

static const uint32_t kTexelSize = 3;
static const uint32_t kTexelCenter = kTexelSize / 2;
static const float32_t kTexelAverage = pow(float32_t(kTexelSize), 2);

float32_t2 PrewittFilter(float32_t2 originalTexcoord,float32_t4x4 inverseMaterial, Texture2D<float32_t> depthTexture ,SamplerState smp){
    uint32_t width, height;
    depthTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
    float32_t2 result = {0.0f, 0.0f};

    for(uint32_t x = 0; x < 3; ++x){
        for(uint32_t y = 0; y < 3; ++y){
            float32_t2 texcoord = originalTexcoord + float32_t2(int32_t(x) - int32_t(kTexelCenter), int32_t(y) - int32_t(kTexelCenter)) * uvStepSize;
            float32_t ndcDepth = depthTexture.Sample(smp, texcoord);
            float32_t4 viewSpace = mul(float32_t4(0.0f,0.0f,ndcDepth,1.0f),inverseMaterial);
            float32_t viewZ=viewSpace.z*rcp(viewSpace.w);
            result.x+=viewZ*kPrewittHorizontalKernel[x][y];
            result.y+=viewZ*kPrewittVerticalKernel[x][y];
        }
    }
    return result;
}