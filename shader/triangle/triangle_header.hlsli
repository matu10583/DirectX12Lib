//‚±‚±‚ÉƒŒƒWƒXƒ^‚ğ‘‚­

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 padding[15];
};

cbuffer SceneConstantBuffer2 : register(b1)
{
    float num;
};
cbuffer SceneConstantBuffer3 : register(b2)
{
    float4x4 mat;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);