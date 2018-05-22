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

#include "LOShaders.hpp"
#include "Util.hpp"
#include <d3dcompiler.h>

#include <string>

#pragma region Help

void ShaderBinder::UnbindAllShaders(ID3D11DeviceContext *dc)
{
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->HSSetShader(nullptr, nullptr, 0);
	dc->DSSetShader(nullptr, nullptr, 0);
	dc->GSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);

	dc->CSSetShader(nullptr, nullptr, 0);
}

void ShaderBinder::UnbindComputeShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(nullptr, nullptr, 0);
}

void ShaderBinder::UnbindPipelineShaders(ID3D11DeviceContext *dc)
{
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->HSSetShader(nullptr, nullptr, 0);
	dc->DSSetShader(nullptr, nullptr, 0);
	dc->GSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
}

void ShaderBinder::UnbindVertexPixelShaders(ID3D11DeviceContext *dc)
{
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
}

#pragma endregion Help

#pragma region ComputeField

ID3D11ComputeShader *ComputeFieldShaders::mComputeFieldShader         = nullptr;
ID3D11ComputeShader *ComputeFieldShaders::mComputeFieldCirclesShader  = nullptr;
ID3D11ComputeShader *ComputeFieldShaders::mComputeFieldRaindopsShader = nullptr;
ID3D11ComputeShader *ComputeFieldShaders::mComputeFieldChainsShader   = nullptr;

bool ComputeFieldShaders::InitAll(ID3D11Device *device)
{
	ID3DBlob *shader;

	std::wstring ShaderPath;

#if defined(DEBUG) || defined(_DEBUG)
	ShaderPath = LR"(..\Debug\Shaders\Compute\)";
#elif defined(NDEBUG)
	ShaderPath = LR"(..\Release\Shaders\Compute\)";
#elif defined(PRODUCT)
	ShaderPath = LR"(Shaders\Compute\)";
#endif

	if(FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeField.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR|MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeFieldShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeFieldCircles.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeFieldCirclesShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeFieldRaindrops.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeFieldRaindopsShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeFieldChains.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeFieldChainsShader));

	return true;
}

void ComputeFieldShaders::DestroyAll()
{
	SafeRelease(mComputeFieldShader);
	SafeRelease(mComputeFieldCirclesShader);
	SafeRelease(mComputeFieldRaindopsShader);
	SafeRelease(mComputeFieldChainsShader);
}

void ComputeFieldShaders::SetComputeFieldShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeFieldShader, nullptr, 0);
}

void ComputeFieldShaders::SetComputeFieldCirclesShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeFieldCirclesShader, nullptr, 0);
}

void ComputeFieldShaders::SetComputeFieldRaindropsShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeFieldRaindopsShader, nullptr, 0);
}

void ComputeFieldShaders::SetComputeFieldChainsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeFieldChainsShader, nullptr, 0);
}

#pragma endregion ComputeField

#pragma region DrawScreen

void* DrawScreenShaders::mInputSignature = nullptr;
int DrawScreenShaders::mInputSignatureSize = 0;

ID3D11VertexShader *DrawScreenShaders::mVertexShader = nullptr;
ID3D11PixelShader  *DrawScreenShaders::mPixelShader = nullptr;

bool DrawScreenShaders::InitAll(ID3D11Device *device)
{
	ID3DBlob *shader;

	std::wstring ShaderPath;

#if defined(DEBUG) || defined(_DEBUG)
	ShaderPath = LR"(..\Debug\Shaders\Render\)";
#elif defined(NDEBUG)
	ShaderPath = LR"(..\Release\Shaders\Render\)";
#elif defined(PRODUCT)
	ShaderPath = LR"(Shaders\Render\)";
#endif

	if(FAILED(D3DReadFileToBlob((ShaderPath + L"VertexShader.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR|MB_OK);
	}

	mInputSignature = shader->GetBufferPointer();
	mInputSignatureSize = shader->GetBufferSize();

	ASSERT_ERR(device->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mVertexShader));

	if(FAILED(D3DReadFileToBlob((ShaderPath + L"PixelShader.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR|MB_OK);
	}
	ASSERT_ERR(device->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mPixelShader));

	return true;
}

void DrawScreenShaders::DestroyAll()
{
	SafeRelease(mVertexShader);
	SafeRelease(mPixelShader);
}

void DrawScreenShaders::SetPipelineShaders(ID3D11DeviceContext *dc)
{
	dc->VSSetShader(mVertexShader, nullptr, 0);
	dc->PSSetShader(mPixelShader, nullptr, 0);
}

#pragma endregion DrawField