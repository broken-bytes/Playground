#pragma pack_matrix(row_major)

struct PointLight {
    float4 position;
    float4 colour;
    float range;
    float2 padding;
};

struct DirectionalLight
{
    float4x4 viewProj;
    float4 direction;
    float4 colour;
};

struct ShadowCaster {
    float4x4 viewProj;
    uint shadowMapIndex;
};

cbuffer Globals: register(b0)
{
    float time;
    float deltaTime;
    float4 position;
    float4 color;
    float intensity;
};

cbuffer DirectionalLightBuffer : register(b1)
{
    DirectionalLight directionalLight;
};

cbuffer CameraBuffer : register(b2)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer MaterialData : register(b3)
{
    int diffuseTextureId;
};

cbuffer ShadowCastersCount: register(b4) {
    uint shadowCastersCount;
};

StructuredBuffer<ShadowCaster> shadowCasters : register(t0);

Texture2D textures[] : register(t1);
SamplerState defaultSampler : register(s0);

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4x4 modelMatrix : INSTANCE_TRANSFORM;
    float4x4 normalMatrix: INSTANCE_NORMALS;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(VSInput vin)
{
    VSOutput vout;

    float4 worldPos = mul(float4(vin.position, 1.0f), vin.modelMatrix);
    float4 viewPos  = mul(worldPos, viewMatrix);
    float4 clipPos  = mul(viewPos, projectionMatrix);

    float3 normalW = mul(vin.normal, (float3x3)vin.normalMatrix);

    vout.position = clipPos;
    vout.color = vin.color;
    vout.normal = normalW;
    vout.uv = vin.uv;

    return vout;
}

float4 PSMain(VSOutput pin) : SV_TARGET
{
    Texture2D diffuseTexture = textures[diffuseTextureId];
    // Sample texture
    float4 texColor = diffuseTexture.Sample(defaultSampler, pin.uv);

    // Directional light setup
    float3 L = normalize(-directionalLight.direction.xyz); // Light coming *toward* surface
    float3 lightColor = directionalLight.colour.rgb;
    float  lightIntensity = directionalLight.colour.a;

    // Surface normal
    float3 N = normalize(pin.normal);

    // Lambertian diffuse lighting
    float NdotL = saturate(dot(N, L));

    // Quantize into 4 bands (0.0, 0.33, 0.66, 1.0)
    float shade = floor(NdotL * 4.0) / 3.0;

    float3 diffuse = lightColor * lightIntensity * shade;

    // Ambient light
    float3 ambient = float3(0.1, 0.1, 0.12);

    // Final color
    float3 finalColor = texColor.rgb * (diffuse + ambient);

    return float4(finalColor, 1.0f);
}
