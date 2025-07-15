cbuffer Screen: register(b0)
{
    float2 inverseScreenRes;
};

cbuffer Globals : register(b1) {
    float time;
}

cbuffer Material : register(b2) {
    uint cubemapId;
}

SamplerState defaultSampler : register(s0);

float4 VSMain(uint id : SV_VertexID) : SV_Position
{
    float2 pos = float2((id == 2) ? 3.0 : -1.0, (id == 1) ? 3.0 : -1.0);
    return float4(pos, 0.0, 1.0);
}

float4 PSMain(float4 position : SV_Position) : SV_Target
{
    float2 uv = position.xy * inverseScreenRes;

    return float4(1, 1, 1, 1);
}
