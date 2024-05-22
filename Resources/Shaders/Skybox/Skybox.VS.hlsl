#include "Skybox.hlsli"

struct VertexShaderInput{
	float32_t4 position : POSITION0;
};

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);
ConstantBuffer<ViewProjection> gViewProjection : register(b1);

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.position= mul(input.position,mul(gWorldTransform.world, mul(gViewProjection.view, gViewProjection.projection))).xyww;
	output.texcoord=input.position.xyz;
	return output;
}