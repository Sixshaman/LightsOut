Texture2D DrawnField: register(t0);

SamplerState SamConst: register(s0);

struct PixelIn
{
	float4 PosH: SV_POSITION;
	float2 Tex:  TEXCOORD;
};

float4 main(PixelIn pin): SV_TARGET
{
	return DrawnField.Sample(SamConst, pin.Tex);
}