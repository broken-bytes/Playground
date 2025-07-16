#pragma pack_matrix(row_major)

cbuffer Screen: register(b0)
{
    float2 inverseScreenRes;
};

cbuffer Camera : register(b1) {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 invViewMatrix;
    float4x4 invProjMatrix;
}

cbuffer Material : register(b2) {
    uint cubemapId;
}

TextureCube cubemaps[] : register(t0, space0);

SamplerState defaultSampler : register(s0);

static const float3 cubeVerts[8] = {
    float3(-1, -1, -1),
    float3(-1,  1, -1),
    float3( 1,  1, -1),
    float3( 1, -1, -1),
    float3(-1, -1,  1),
    float3(-1,  1,  1),
    float3( 1,  1,  1),
    float3( 1, -1,  1),
};

static const uint cubeIndices[36] = {
    // Front face
    0, 1, 2, 0, 2, 3,
    // Back face
    4, 6, 5, 4, 7, 6,
    // Left
    4, 5, 1, 4, 1, 0,
    // Right
    3, 2, 6, 3, 6, 7,
    // Top
    1, 5, 6, 1, 6, 2,
    // Bottom
    4, 0, 3, 4, 3, 7,
};

struct VSOut {
    float4 position : SV_Position;
    float3 dir : TEXCOORD0;
};

VSOut VSMain(uint vertexID : SV_VertexID)
{
    VSOut o;

    float3 pos = cubeVerts[cubeIndices[vertexID]];

    // Strip translation from view
    float4x4 viewNoTrans = viewMatrix;
    viewNoTrans[3] = float4(0, 0, 0, 1);

    float4 worldPos = mul(float4(pos, 1.0), viewNoTrans);
    o.dir = pos;

    o.position = mul(worldPos, projectionMatrix);
    return o;
}

float4 PSMain(VSOut input) : SV_Target
{
    return cubemaps[cubemapId].Sample(defaultSampler, input.dir);
}
