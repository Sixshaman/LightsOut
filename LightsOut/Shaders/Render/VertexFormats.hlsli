struct VertexIn
{
	float3 Pos: POSITION;
	float2 Tex: TEXCOORD;
};

struct VertexOut
{
	float4 PosH: SV_POSITION;
	float2 Tex: TEXCOORD;
};