#include "RenderStates.hpp"
#include "..\Util.hpp"

RenderStates::RenderStates(ID3D11Device *device)
{
	D3D11_SAMPLER_DESC SamplerPointDesc;
	ZeroMemory(&SamplerPointDesc, sizeof(D3D11_SAMPLER_DESC));
	SamplerPointDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	SamplerPointDesc.MaxAnisotropy = 4;
	SamplerPointDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerPointDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerPointDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	ThrowIfFailed(device->CreateSamplerState(&SamplerPointDesc, mTexturePointSamplerState.GetAddressOf()));
}

RenderStates::~RenderStates()
{
}