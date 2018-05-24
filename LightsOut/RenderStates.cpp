#include "RenderStates.hpp"
#include "Util.hpp"

ID3D11SamplerState *RenderStates::mTextureSS = nullptr;

bool RenderStates::InitAll(ID3D11Device *device)
{
	D3D11_SAMPLER_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_SAMPLER_DESC));
	TextureDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	TextureDesc.MaxAnisotropy = 4;
	TextureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	TextureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	TextureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	ASSERT_ERR(device->CreateSamplerState(&TextureDesc, &mTextureSS));

	return true;
}

void RenderStates::DestroyAll()
{
	SafeRelease(mTextureSS);
}