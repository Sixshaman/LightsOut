#include "VertexFormats.hlsli"

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = float4(vin.Pos, 1.0f);
	vout.Tex = vin.Tex;

	return vout;
}