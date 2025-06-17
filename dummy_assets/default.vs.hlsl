struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4x4 modelMatrix : INSTANCE_TRANSFORM;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

// Constant buffer for camera VP matrix
cbuffer CameraBuffer : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};


VSOutput VSMain(VSInput vin)
{
    VSOutput vout;

    // Load the Model matrix for this instance

    // Transform the position
    float4 worldPos = mul(vin.modelMatrix, float4(vin.position, 1));
    float4 viewPos = mul(viewMatrix, worldPos);
    float4 colour = vin.color;

    vout.position = mul(projectionMatrix, viewPos);
    vout.color = colour;
    vout.normal = vin.normal;
    vout.uv = vin.uv;

    return vout;
}
