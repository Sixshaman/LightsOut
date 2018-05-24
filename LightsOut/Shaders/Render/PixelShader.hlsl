#include "VertexFormats.hlsli"

Texture2D DrawedField: register(t0);

SamplerState SamConst: register(s0);

float4 main(VertexOut vout): SV_TARGET
{
	return DrawedField.Sample(SamConst, vout.Tex);
}