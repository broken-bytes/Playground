struct PSInput {
    float4 position : SV_Position;
    float4 color    : COLOR;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
