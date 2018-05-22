/*
Copyright (c) 2015 Sixshaman

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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