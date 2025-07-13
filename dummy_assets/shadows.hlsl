#pragma pack_matrix(row_major)

struct DirectionalLight
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
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
    // Transform world position to light clip space
    float4 lightViewPos = mul(worldPos, directionalLight.viewMatrix);
    float4 clipPos = mul(lightViewPos, directionalLight.projectionMatrix);

    output.position = clipPos;

    return output;
}

void PSMain() {}

