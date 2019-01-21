#include "LORenderer.h"
#include "RenderStates.hpp"
#include <DirectXColors.h>
#include "LOShaders.hpp"
#include "LOTextures.hpp"
#include "LOShaderVariables.hpp"
#include "..\Util.hpp"

LightsOutRenderer::LightsOutRenderer()
{
	mDrawType = DrawType::DRAW_SQUARES;
}

LightsOutRenderer::~LightsOutRenderer()
{
	DrawScreenShaders::DestroyAll();
	DrawScreenVariables::DestroyAll();
	ComputeBoardShaders::DestroyAll();
	ComputeBoardVariables::DestroyAll();
	RenderStates::DestroyAll();
	LOTextures::DestroyAll();
}

bool LightsOutRenderer::InitD3D(HWND hwnd)
{
	uint32_t flags = 0;
	D3D_FEATURE_LEVEL featurelvl;

#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	RECT R;
	GetClientRect(hwnd, &R);
	uint16_t wndWidth  = R.right - R.left;
	uint16_t wndHeight = R.bottom - R.top;

	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.Width = wndWidth;
	scDesc.BufferDesc.Height = wndHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 1;
	scDesc.OutputWindow = hwnd;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = 0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
		                                       &scDesc, &mSwapChain, &md3dDevice, &featurelvl, &md3dContext);

	if (FAILED(hr))
	{
		MessageBox(nullptr, L"D3D11 device creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if (featurelvl < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(nullptr, L"D3D11 is not supported!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if (!OnWndResize(wndWidth, wndHeight))
	{
		return false;
	}

	if (!DrawScreenShaders::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!ComputeBoardShaders::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!DrawScreenVariables::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!ComputeBoardVariables::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!RenderStates::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!LOTextures::InitSRVs(md3dDevice.Get()))
	{
		return false;
	}

	SetColorTheme(ColorTheme::NEON_XXL);
	EdgeColorAsUnlit();

	return true;
}

void LightsOutRenderer::ResetBoardSize(uint16_t newSize)
{
	uint32_t cellSize = (uint32_t)(ceilf(EXPECTED_WND_SIZE / newSize) - 1);
	ComputeBoardVariables::SetCellSize(cellSize);

	ComputeBoardVariables::SetBoardSize(newSize);
	LOTextures::ResizeBoard(newSize, cellSize, md3dDevice.Get());
}

void LightsOutRenderer::SetSolutionVisible(bool visible)
{
	ComputeBoardVariables::SetSolutionVisible(visible);
}

void LightsOutRenderer::SetStabilityVisible(bool visible)
{
	ComputeBoardVariables::SetStabilityVisible(visible);
}

bool LightsOutRenderer::OnWndResize(uint16_t newWidth, uint16_t newHeight)
{
	mRenderTarget.Reset();
	mDepthStencilView.Reset();

	if(FAILED(mSwapChain->ResizeBuffers(1, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
	{
		MessageBox(nullptr, L"Buffers resizing error", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));

	if (FAILED(md3dDevice->CreateRenderTargetView(backBuffer.Get(), NULL, mRenderTarget.GetAddressOf())))
	{
		MessageBox(nullptr, L"Render target creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	depthStencilBufferDesc.Width = newWidth;
	depthStencilBufferDesc.Height = newHeight;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;

	ID3D11Texture2D *depthStencilBuffer = nullptr;
	if (FAILED(md3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &depthStencilBuffer)))
	{
		MessageBox(nullptr, L"Depth buffer creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}
	if (FAILED(md3dDevice->CreateDepthStencilView(depthStencilBuffer, nullptr, &mDepthStencilView)))
	{
		MessageBox(nullptr, L"Depth/stencil view creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	ID3D11RenderTargetView* renderTargets[] = {mRenderTarget.Get()};
	md3dContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView.Get());

	mViewport.Width    = (float)newWidth;
	mViewport.Height   = (float)newHeight;
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	md3dContext->RSSetViewports(1, &mViewport);

	return true;
}

void LightsOutRenderer::SetDrawType(DrawType drawType)
{
	mDrawType = drawType;
}

void LightsOutRenderer::SetBoardToDraw(const LightsOutBoard& board)
{
	LOTextures::UpdateBoard(board, md3dContext.Get());
}

void LightsOutRenderer::SetSolutionToDraw(const LightsOutBoard& solution)
{
	LOTextures::UpdateSolution(solution, md3dContext.Get());
}

void LightsOutRenderer::SetStabilityToDraw(const LightsOutBoard& stability)
{
	LOTextures::UpdateStability(stability, md3dContext.Get());
}

void LightsOutRenderer::SetColorTheme(ColorTheme colorTheme)
{
	switch (colorTheme)
	{
	case ColorTheme::RED_EXPLOSION:
		ComputeBoardVariables::SetColorSolved(DirectX::Colors::Red);
		ComputeBoardVariables::SetColorEnabled(DirectX::Colors::Crimson);
		ComputeBoardVariables::SetColorNone(DirectX::Colors::Salmon);
		break;
	case ColorTheme::NEON_XXL:
		ComputeBoardVariables::SetColorSolved(DirectX::Colors::Blue);
		ComputeBoardVariables::SetColorEnabled(DirectX::Colors::Lime);
		ComputeBoardVariables::SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::CREAMED_STRAWBERRY:
		ComputeBoardVariables::SetColorSolved(DirectX::Colors::Chocolate);
		ComputeBoardVariables::SetColorEnabled(DirectX::Colors::HotPink);
		ComputeBoardVariables::SetColorNone(DirectX::Colors::Bisque);
		break;
	case ColorTheme::HARD_TO_SEE:
		ComputeBoardVariables::SetColorSolved(DirectX::Colors::MidnightBlue);
		ComputeBoardVariables::SetColorEnabled(DirectX::Colors::Navy);
		ComputeBoardVariables::SetColorNone(DirectX::Colors::DarkBlue);
		break;
	case ColorTheme::BLACK_AND_WHITE:
		ComputeBoardVariables::SetColorSolved(DirectX::Colors::DimGray);
		ComputeBoardVariables::SetColorEnabled(DirectX::Colors::White);
		ComputeBoardVariables::SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::PETYA:
		ComputeBoardVariables::SetColorSolved(DirectX::XMVectorSet(0.639f, 0.694f, 0.745f, 1.0f));
		ComputeBoardVariables::SetColorEnabled(DirectX::XMVectorSet(0.980f, 0.984f, 0.988f, 1.0f));
		ComputeBoardVariables::SetColorNone(DirectX::XMVectorSet(0.459f, 0.733f, 0.992f, 1.0f));
		break;
	default:
		break;
	}
}

void LightsOutRenderer::EdgeColorAsUnlit()
{
	ComputeBoardVariables::SetColorBetweenAsNone();
}

void LightsOutRenderer::EdgeColorAsLit()
{
	ComputeBoardVariables::SetColorBetweenAsEnabled();
}

void LightsOutRenderer::EdgeColorAsSolution()
{
	ComputeBoardVariables::SetColorBetweenAsSolved();
}

void LightsOutRenderer::EdgeColorDimmed()
{
	ComputeBoardVariables::SetColorBetweenAsDimmed();
}

void LightsOutRenderer::DrawBoardOnTexture(uint16_t cellSize, uint16_t gameSize)
{
	FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	md3dContext->ClearUnorderedAccessViewFloat(LOTextures::ResultUAV(), clearColor);

	//--------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------Draw the board on the off-screen texture------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------

	ComputeBoardVariables::UpdateCSCBuffer(md3dContext.Get());
	ComputeBoardVariables::SetCSVariables(md3dContext.Get());

	switch (mDrawType)
	{
	case DrawType::DRAW_SQUARES:
		ComputeBoardShaders::SetComputeBoardShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CIRCLES:
		ComputeBoardShaders::SetComputeBoardCirclesShader(md3dContext.Get());
		break;
	case DrawType::DRAW_RAINDROPS:
		ComputeBoardShaders::SetComputeBoardRaindropsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CHAINS:
		ComputeBoardShaders::SetComputeBoardChainsShader(md3dContext.Get());
		break;
	default:
		ComputeBoardShaders::SetComputeBoardShader(md3dContext.Get());
		break;
	}

	md3dContext->Dispatch((uint32_t)ceilf(gameSize * (cellSize / 16.0f)),
		                  (uint32_t)ceilf(gameSize * (cellSize / 16.0f)),
		                  1);

	ComputeBoardVariables::DisableVariables(md3dContext.Get());
	ShaderBinder::UnbindComputeShader(md3dContext.Get());
}

void LightsOutRenderer::DrawBoardTexOnScreen()
{
	FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	md3dContext->ClearRenderTargetView(mRenderTarget.Get(), clearColor);
	md3dContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	md3dContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);

	md3dContext->IASetInputLayout(nullptr);

	md3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	DrawScreenVariables::SetAllVariables(md3dContext.Get());
	DrawScreenShaders::SetPipelineShaders(md3dContext.Get());

	md3dContext->Draw(4, 0);

	DrawScreenVariables::DisableVariables(md3dContext.Get());
	ShaderBinder::UnbindPipelineShaders(md3dContext.Get());
}

bool LightsOutRenderer::IsDrawTypeBinary()
{
	return (mDrawType == DrawType::DRAW_SQUARES) || (mDrawType == DrawType::DRAW_CIRCLES) || (mDrawType == DrawType::DRAW_RAINDROPS) || (mDrawType == DrawType::DRAW_CHAINS);
}

bool LightsOutRenderer::IsDrawTypeDomain()
{
	return mDrawType == DrawType::DRAW_SQUARES_DOMAIN;
}

bool LightsOutRenderer::IsDrawTypeSquares()
{
	return (mDrawType == DrawType::DRAW_SQUARES) || (mDrawType == DrawType::DRAW_SQUARES_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeCircles()
{
	return mDrawType == DrawType::DRAW_CIRCLES;
}

bool LightsOutRenderer::IsDrawTypeRaindrops()
{
	return mDrawType == DrawType::DRAW_RAINDROPS;
}

bool LightsOutRenderer::IsDrawTypeChains()
{
	return mDrawType == DrawType::DRAW_CHAINS;
}

void LightsOutRenderer::DrawBoard(uint16_t cellSize, uint16_t gameSize)
{
	DrawBoardOnTexture(cellSize, gameSize);
	DrawBoardTexOnScreen();

	mSwapChain->Present(0, 0);
}

void LightsOutRenderer::DrawBgBoardToMemory(uint16_t cellSize, uint16_t gameSize, std::vector<uint32_t>& outData, uint32_t& outRowPitch)
{
	outData.clear();

	ComputeBoardVariables::SetCellSize(cellSize);

	LOTextures::ResizeBoard(gameSize, cellSize, md3dDevice.Get());
	DrawBoardOnTexture(cellSize, gameSize);

	ID3D11Texture2D* resultTex = LOTextures::MappedTex(md3dContext.Get());

	D3D11_TEXTURE2D_DESC tDesc;
	resultTex->GetDesc(&tDesc);

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	md3dContext->Map(resultTex, 0, D3D11_MAP_READ, 0, &mappedRes);
	uint32_t* data = reinterpret_cast<uint32_t*>(mappedRes.pData);
	outData.resize(mappedRes.RowPitch * tDesc.Height / sizeof(uint32_t));
	std::copy(&data[0], &data[outData.size()], outData.begin());
	md3dContext->Unmap(resultTex, 0);

	outRowPitch = mappedRes.RowPitch;
}