cbuffer Screen: register(b1) {
    float2 inverseScreenRes;
};

cbuffer Camera : register(b2) {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 invViewMatrix;
    float4x4 invProjMatrix;
}

cbuffer Material : register(b3)
{
    float intensity;
};

Texture2D colourBuffer : register(t0, space0);
Texture2D depthBuffer : register(t0, space1);
SamplerState defaultSampler : register(s0);

struct VSOutput {
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    float2 positions[3] = {
        float2(-1, -1),
        float2(-1, 3),
        float2(3, -1)
    };
    float2 uvs[3] = {
        float2(0, 1),
        float2(0, -1),
        float2(2, 1)
    };
    VSOutput output;
    output.pos = float4(positions[vertexID], 0, 1);
    output.uv = float2((positions[vertexID].x + 1) * 0.5, 1.0 - (positions[vertexID].y + 1) * 0.5); // Y-flip for DX!

    return output;
}

float LinearizeDepth(float depth)
{
    float n = 10.0;   // near plane
    float f = 500.0;  // far plane
    float z = depth * 2.0 - 1.0; // Convert [0,1] to [-1,1]
    return (2.0 * n * f) / (f + n - z * (f - n));
}

float4 PSMain(VSOutput input) : SV_Target
{
    float2 uv = saturate(input.uv);
    float depth = depthBuffer.Sample(defaultSampler, uv).r;
    float linearDepth = LinearizeDepth(depth);

    float fogStart = 80.0;
    float fogEnd = 500.0;
    float fogFactor = 1.0 - exp(-linearDepth / 1000.0);

    float3 fogColor = float3(0.3, 0.3, 0.3);

    float3 color = colourBuffer.Sample(defaultSampler, uv).rgb;
    float3 finalColor = lerp(color, fogColor, fogFactor);

    return float4(finalColor, 1);
}
