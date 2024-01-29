
float HalfRanbert(float3 normal, float3 lightDirection)
{
    float NdotL = saturate(dot(normalize(normal), -lightDirection));
    return pow(NdotL * 0.5f + 0.5f, 2.0f);
}

float Ranbert(float3 normal, float3 lightDirection)
{
    return saturate(dot(normalize(normal), -lightDirection));
}

float BlinnPhongReflection(float3 normal, float3 cameraPos, float3 worldPos, float3 lightDirection)
{
    float3 toEye = normalize(cameraPos - worldPos);
    float3 halfVector = normalize(-lightDirection + toEye);
    float NDotH = dot(normalize(normal), halfVector);
    
    return saturate(NDotH);
}

float3 PhongReflection(float3 normal, float3 cameraPos, float3 worldPos, float3 lightDirection)
{
    float3 refVec = reflect(-lightDirection, normalize(normal));
    float3 toEye = normalize(cameraPos - worldPos);
    return saturate(dot(refVec, toEye));
}

float3 PointLightDirection(float3 worldPos, float3 pointLightPos)
{
    return normalize(worldPos - pointLightPos);
}

float Factor(float3 worldPos, float3 pointLightPos,float radius,float decay)
{
    float distance = length((pointLightPos - worldPos));
    
    return pow(saturate(-distance / radius + 1.0f), decay);

}

float PointLightHalfRanbert(float3 normal, float3 worldPos, float3 pointLightPos)
{
    float NdotL = saturate(dot(normalize(normal), -PointLightDirection(worldPos, pointLightPos)));
    return pow(NdotL * 0.5f + 0.5f, 2.0f);
}

float3 PointLightRanbert(float3 normal, float3 worldPos, float3 pointLightPos)
{
    return saturate(dot(normalize(normal), -PointLightDirection(worldPos, pointLightPos)));
}

float PointLightBlinnPhongReflection(float3 normal, float3 worldPos, float3 pointLightPos)
{
    float3 toEye = normalize(pointLightPos - worldPos);
    float3 halfVector = normalize(-PointLightDirection(worldPos, pointLightPos) + toEye);
    float NDotH = dot(normalize(normal), halfVector);
    
    return saturate(NDotH);
}

float PointLightPhongReflection(float3 normal, float3 worldPos, float3 pointLightPos)
{
    float3 refVec = reflect(-PointLightDirection(worldPos, pointLightPos), normalize(normal));
    float3 toEye = normalize(pointLightPos - worldPos);
    return saturate(dot(refVec, toEye));
}