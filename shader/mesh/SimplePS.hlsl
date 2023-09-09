struct MSOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

struct PSOutput
{
    float4 Color : SV_Target0;
};

PSOutput main(MSOutput input)
{
    PSOutput output = (PSOutput) 0;
    output.Color = input.Color;
	return output;
}