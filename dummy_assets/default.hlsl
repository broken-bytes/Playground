#pragma pack_matrix(row_major)

struct PointLight {
    float4 position;
    float4 colour;
    float range;
    float2 padding;
};

struct DirectionalLight
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 direction;
    float4 colour;
};

struct ShadowCaster {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
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
SamplerComparisonState shadowSampler : register(s0);
SamplerState defaultSampler : register(s1);

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4x4 modelMatrix : INSTANCE_TRANSFORM;
    float4x4 normalMatrix: INSTANCE_NORMALS;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 worldPos : TEXCOORD1;
};

float SampleShadowMap(float4 worldPos, ShadowCaster sc)
{
    // 1) project into light-space
    float4 scView = mul(worldPos, sc.viewMatrix);
    float4 scProj = mul(scView,  sc.projectionMatrix);

    if (scProj.w <= 0.0f) return 1.0f;

    float2 uv = float2(scProj.x, -scProj.y) * 0.5f + 0.5f;    
    float depth = scProj.z + 0.001f;
    // 3) only sample if inside this cascade’s bounds
    if (uv.x < 0 || uv.x > 1 ||
        uv.y < 0 || uv.y > 1)
    {
        // outside, treat as fully lit
        return 1.0f;
    }

    // 4) do the comparison
    // no clamping on uv or depth here
    return textures[sc.shadowMapIndex].SampleCmpLevelZero(shadowSampler, uv, depth);
}

VSOutput VSMain(VSInput vin)
{
    VSOutput vout;

    float4 worldPos = mul(float4(vin.position, 1.0f), vin.modelMatrix);
    float4 viewPos  = mul(worldPos, viewMatrix);
    float4 clipPos  = mul(viewPos, projectionMatrix);

    float3 normalW = mul(vin.normal, (float3x3)vin.normalMatrix);

    vout.position = clipPos;
    vout.worldPos = worldPos;
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

    float shadowFactor = 1.0f;
    for (uint i = 0; i < shadowCastersCount; ++i)
    {
        shadowFactor *= SampleShadowMap(pin.worldPos, shadowCasters[i]);
    }

    // Surface normal
    float3 N = normalize(pin.normal);

    // Lambertian diffuse lighting
    float NdotL = saturate(dot(N, L));

    float shade = NdotL;

    float3 diffuse = lightColor * lightIntensity * shade * shadowFactor;

    // Ambient light
    float3 ambient = float3(0.1, 0.1, 0.12);

    // Final color
    float3 finalColor = texColor.rgb * (diffuse + ambient);


    return float4(finalColor, 1.0f);
}
