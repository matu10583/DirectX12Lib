struct MSInput
{
    float3 Position;
    float4 Color;
};

struct MSOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

struct TransformParam
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};

StructuredBuffer<MSInput> Vertices : register(t0);
StructuredBuffer<uint3> Indices : register(t1);
ConstantBuffer<TransformParam> Transform : register(b0);

[numthreads(64,1,1)]
[outputtopology("triangle")]
void main(
uint groupIndex: SV_GroupIndex,
out vertices MSOutput verts[3],
out indices uint3 tris[1]
){
    //スレッドグループの頂点とプリみぃてぃぶの数を設定
    SetMeshOutputCounts(3, 1);
    if (groupIndex < 1)
    {
        tris[groupIndex] = Indices[groupIndex];
    }
    
    if (groupIndex < 3)
    {
        MSOutput output1 = (MSOutput) 0;
        float4 localPos = float4(Vertices[groupIndex].Position, 1.0f);
        float4 worldPos = mul(Transform.World, localPos);
        float4 viewPos = mul(Transform.View, worldPos);
        float4 projPos = mul(Transform.Proj, viewPos);
        output1.Position = projPos;
        output1.Color = Vertices[groupIndex].Color;
        verts[groupIndex] = output1;
    }
}