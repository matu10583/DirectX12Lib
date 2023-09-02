#include "triangle_header.hlsli"


PSInput VSmain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    //result.position = mul(offset, mat);
    result.position = position;
    
    result.color = float4(0, 0, 0, num);

    return result;
}