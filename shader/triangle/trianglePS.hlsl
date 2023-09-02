#include "triangle_header.hlsli"

float4 PSmain(PSInput input) : SV_TARGET0
{
    float4 col = g_texture.Sample(g_sampler, float2(0.0, 0.0));
    //col += offset;
    return col;
}