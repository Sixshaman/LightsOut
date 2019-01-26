#include "LOShaders.hpp"
#include "..\Util.hpp"
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

#pragma region ComputeBoard

ID3D11ComputeShader* ComputeBoardShaders::mComputeBoardShader               = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeBoardCirclesShader        = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeBoardRaindopsShader       = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeBoardChainsShader         = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeEveryBoardShader          = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeEveryBoardCirclesShader   = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeEveryBoardRaindropsShader = nullptr;
ID3D11ComputeShader* ComputeBoardShaders::mComputeEveryBoardChainsShader    = nullptr;

bool ComputeBoardShaders::InitAll(ID3D11Device *device)
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

	if(FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeBoard.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR|MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeBoardShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeBoardCircles.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeBoardCirclesShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeBoardRaindrops.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeBoardRaindopsShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeBoardChains.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeBoardChainsShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoard.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeEveryBoardShader));

	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardCircles.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeEveryBoardCirclesShader));
	
	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardRaindrops.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeEveryBoardRaindropsShader));
	
	if (FAILED(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardChains.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR | MB_OK);
	}

	ASSERT_ERR(device->CreateComputeShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mComputeEveryBoardChainsShader));

	return true;
}

void ComputeBoardShaders::DestroyAll()
{
	SafeRelease(mComputeBoardShader);
	SafeRelease(mComputeBoardCirclesShader);
	SafeRelease(mComputeBoardRaindopsShader);
	SafeRelease(mComputeBoardChainsShader);
	SafeRelease(mComputeEveryBoardShader);
	SafeRelease(mComputeEveryBoardCirclesShader);
	SafeRelease(mComputeEveryBoardRaindropsShader);
	SafeRelease(mComputeEveryBoardChainsShader);
}

void ComputeBoardShaders::SetComputeBoardShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeBoardShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardCirclesShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeBoardCirclesShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardRaindropsShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeBoardRaindopsShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardChainsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeBoardChainsShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardCirclesShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardCirclesShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardRaindropsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardRaindropsShader, nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardChainsShader(ID3D11DeviceContext * dc)
{
	dc->CSSetShader(mComputeEveryBoardChainsShader, nullptr, 0);
}

#pragma endregion ComputeBoard

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

#pragma endregion DrawBoard
