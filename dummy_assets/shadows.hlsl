#pragma pack_matrix(row_major)

struct DirectionalLight
{
    float4x4 viewProj;
    float4 direction;
    float4 colour;
};

cbuffer DirectionalLightBuffer : register(b0)
{
    DirectionalLight directionalLight;
};

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4x4 modelMatrix : INSTANCE_TRANSFORM;
    float4x4 normalMatrix: INSTANCE_NORMALS;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.position, 1.0f), input.modelMatrix);
    float4 clipPos = mul(worldPos, directionalLight.viewProj);
    output.position = clipPos;

    return output;
}

float PSMain() : SV_DEPTH
{
    return 0.0f;
}
