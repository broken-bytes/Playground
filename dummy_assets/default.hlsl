struct PointLight {
    float4 position;
    float4 colour;
    float range;
    float2 padding;
};

struct DirectionalLight
{
    float4 direction;
    float4 colour;
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
    float4 windPos;
    float4 windDir;
};

StructuredBuffer<PointLight> lights : register(t0);

Texture2D diffuse : register(t1);
SamplerState defaultSampler : register(s0);

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

VSOutput VSMain(VSInput vin)
{
    VSOutput vout;

    // Load the Model matrix for this instance

    // Transform the position
    float4 worldPos = mul(vin.modelMatrix, float4(vin.position, 1));
    float4 viewPos = mul(viewMatrix, worldPos);
    float4 colour = vin.color;
    float3x3 normalMatrix = (float3x3)vin.modelMatrix;
    float3 normalW = mul(normalMatrix, vin.normal);

    vout.position = mul(projectionMatrix, viewPos);
    vout.color = colour;
    vout.normal = normalW;
    vout.uv = vin.uv;

    return vout;
}

float4 PSMain(VSOutput pin) : SV_TARGET
{
    // Sample texture
    float4 texColor = diffuse.Sample(defaultSampler, pin.uv);

    // Directional light setup
    float3 L = normalize(-directionalLight.direction.xyz); // Light coming *toward* surface
    float3 lightColor = directionalLight.colour.rgb;
    float  lightIntensity = directionalLight.colour.a; // .w from your glm vec4

    // Surface normal
    float3 N = normalize(pin.normal);

    // Diffuse wrap lighting
    float wrap = 0.5 + 0.5 * dot(N, L);
    float softDiffuse = pow(saturate(wrap), 1.5); // Clamp to avoid NaNs

    float3 diffuse = lightColor * lightIntensity * softDiffuse;

    // Ambient light (always on)
    float3 ambient = float3(0.1, 0.1, 0.12);

    // Final lit color
    float3 finalColor = pin.uv.xyx * (diffuse + ambient);


    // Output color — remove wind scaling for now
    return float4(finalColor, 1);
}
