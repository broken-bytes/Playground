struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;

    // Instance data (slot 1, per instance)
    float4x4 modelMatrix : INSTANCE_TRANSFORM; // Use a custom semantic or just pass as 4 float4 attributes
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(VSInput input, uint instanceID : SV_InstanceID) {
    VSOutput output;

    // Transform position by model matrix
    float4 worldPos = mul(float4(input.position, 1.0), input.modelMatrix);

    // Then your existing logic with worldPos instead of input.position
    output.position = worldPos;
    output.color = input.color;

    return output;
}
