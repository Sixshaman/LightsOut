#include "LOShaders.hpp"
#include "..\Util.hpp"
#include <d3dcompiler.h>

#include <string>

#pragma region ComputeBoard

ComputeBoardShaders::ComputeBoardShaders(ID3D11Device *device)
{
	std::wstring ShaderPath;
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;

#if defined(DEBUG) || defined(_DEBUG)
	ShaderPath = LR"(..\Debug\Shaders\Compute\)";
#elif defined(NDEBUG)
	ShaderPath = LR"(Shaders\Compute\)";
#elif defined(PRODUCT)
	ShaderPath = LR"(Shaders\Compute\)";
#endif

	ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeBoard.cso").c_str(), shaderBlob.GetAddressOf()));
	ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeBoardShader.GetAddressOf()));

	ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeBoardCircles.cso").c_str(), shaderBlob.GetAddressOf()));
	ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeBoardCirclesShader.GetAddressOf()));

	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeBoardDiamonds.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeBoardDiamondsShader.GetAddressOf()));

	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeBoardBeams.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeBoardBeamsShader.GetAddressOf()));

	ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeBoardRaindrops.cso").c_str(), shaderBlob.GetAddressOf()));
	ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeBoardRaindopsShader.GetAddressOf()));

	ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeBoardChains.cso").c_str(), shaderBlob.GetAddressOf()));
	ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeBoardChainsShader.GetAddressOf()));

	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoard.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeEveryBoardShader.GetAddressOf()));

	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardCircles.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeEveryBoardCirclesShader.GetAddressOf()));
	//
	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardDiamonds.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeEveryBoardDiamondsShader.GetAddressOf()));

	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardBeams.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeEveryBoardBeamsShader.GetAddressOf()));

	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardRaindrops.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeEveryBoardRaindropsShader.GetAddressOf()));
	//
	//ThrowIfFailed(D3DReadFileToBlob((ShaderPath + L"ComputeEveryBoardChains.cso").c_str(), shaderBlob.GetAddressOf()));
	//ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, mComputeEveryBoardChainsShader.GetAddressOf()));
}

ComputeBoardShaders::~ComputeBoardShaders()
{
}

void ComputeBoardShaders::SetComputeBoardShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeBoardShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardCirclesShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeBoardCirclesShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardDiamondsShader(ID3D11DeviceContext * dc)
{
	dc->CSSetShader(mComputeBoardDiamondsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardBeamsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeBoardBeamsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardRaindropsShader(ID3D11DeviceContext *dc)
{
	dc->CSSetShader(mComputeBoardRaindopsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeBoardChainsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeBoardChainsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardCirclesShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardCirclesShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardDiamondsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardDiamondsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardBeamsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardBeamsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardRaindropsShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(mComputeEveryBoardRaindropsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::SetComputeEveryBoardChainsShader(ID3D11DeviceContext * dc)
{
	dc->CSSetShader(mComputeEveryBoardChainsShader.Get(), nullptr, 0);
}

void ComputeBoardShaders::UnbindComputeShader(ID3D11DeviceContext* dc)
{
	dc->CSSetShader(nullptr, nullptr, 0);
}

#pragma endregion ComputeBoard

#pragma region DrawScreen

DrawScreenShaders::DrawScreenShaders(ID3D11Device *device)
{
	ID3DBlob *shader;

	std::wstring ShaderPath;

#if defined(DEBUG) || defined(_DEBUG)
	ShaderPath = LR"(..\Debug\Shaders\Render\)";
#elif defined(NDEBUG)
	ShaderPath = LR"(Shaders\Render\)";
#elif defined(PRODUCT)
	ShaderPath = LR"(Shaders\Render\)";
#endif

	if(FAILED(D3DReadFileToBlob((ShaderPath + L"VertexShader.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR|MB_OK);
	}

	ThrowIfFailed(device->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));

	if(FAILED(D3DReadFileToBlob((ShaderPath + L"PixelShader.cso").c_str(), &shader)))
	{
		MessageBox(NULL, L"Cannot read shader file", L"Error", MB_ICONERROR|MB_OK);
	}
	ThrowIfFailed(device->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));
}

DrawScreenShaders::~DrawScreenShaders()
{
}

void DrawScreenShaders::SetPipelineShaders(ID3D11DeviceContext *dc)
{
	dc->VSSetShader(mVertexShader.Get(), nullptr, 0);
	dc->PSSetShader(mPixelShader.Get(),  nullptr, 0);
}

void DrawScreenShaders::UnbindPipelineShaders(ID3D11DeviceContext* dc)
{
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
}

#pragma endregion DrawBoard
