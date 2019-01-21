static const float2 ScreenPos[4] =
{
	float2(-1.0f,  1.0f),
	float2( 1.0f,  1.0f),
	float2(-1.0f, -1.0f),
	float2( 1.0f, -1.0f)
};

static const float2 ScreenTex[] =
{
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
};

struct VertexOut
{
	float4 PosH: SV_POSITION;
	float2 Tex:  TEXCOORD;
};

VertexOut main(uint VertexID: SV_VertexID)
{
	VertexOut vout;

	vout.PosH = float4(ScreenPos[VertexID], 0.0f, 1.0f);
	vout.Tex  = ScreenTex[VertexID];

	return vout;
}