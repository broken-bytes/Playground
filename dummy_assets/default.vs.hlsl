struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    // Set the output position
    output.position = float4(input.position, 1);
    // Pass through other attributes
    output.color = input.color;
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}
