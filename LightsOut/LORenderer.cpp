#include "LORenderer.h"
#include "RenderStates.hpp"
#include <DirectXColors.h>
#include "LOShaders.hpp"
#include "LOTextures.hpp"
#include "LOShaderVariables.hpp"

LightsOutRenderer::LightsOutRenderer()
{
	mDrawType = DrawType::DRAW_SQUARES;
}

LightsOutRenderer::~LightsOutRenderer()
{
	DrawScreenShaders::DestroyAll();
	DrawScreenVariables::DestroyAll();
	ComputeFieldShaders::DestroyAll();
	ComputeFieldVariables::DestroyAll();
	RenderStates::DestroyAll();
	LOTextures::DestroyAll();
}

bool LightsOutRenderer::InitD3D(HWND hwnd)
{
	uint32_t flags = 0;
	D3D_FEATURE_LEVEL featurelvl;

#if defined(DEBUG) || defined(_DEBUG)
	flags | D3D11_CREATE_DEVICE_DEBUG;
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

	if (!ComputeFieldShaders::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!DrawScreenVariables::InitAll(md3dDevice.Get()))
	{
		return false;
	}

	if (!ComputeFieldVariables::InitAll(md3dDevice.Get()))
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

	ComputeFieldVariables::SetColorSolved(DirectX::Colors::Blue);
	ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Lime);
	ComputeFieldVariables::SetColorNone(DirectX::Colors::Black);
	ComputeFieldVariables::SetColorBetween(DirectX::Colors::DarkGray);

	return true;
}

void LightsOutRenderer::ResetFieldSize(uint16_t newSize)
{
	ComputeFieldVariables::SetFieldSize(newSize);
	LOTextures::ResizeField(newSize, md3dDevice.Get());
}

void LightsOutRenderer::SetCellSize(uint16_t cellSize)
{
	ComputeFieldVariables::SetCellSize(cellSize);
}

void LightsOutRenderer::SetSolutionVisible(bool visible)
{
	ComputeFieldVariables::SetSolutionVisible(visible);
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

void LightsOutRenderer::SetFieldBufferData(boost::dynamic_bitset<uint32_t> field)
{
	LOTextures::UpdateField(field, md3dContext.Get());
}

void LightsOutRenderer::SetSolutionBufferData(boost::dynamic_bitset<uint32_t> solution)
{
	LOTextures::UpdateSolution(solution, md3dContext.Get());
}

void LightsOutRenderer::SetColorTheme(ColorTheme colorTheme)
{
	switch (colorTheme)
	{
	case ColorTheme::RED_EXPLOSION:
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Red);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Crimson);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Salmon);
		break;
	case ColorTheme::NEON_XXL:
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Blue);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Lime);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::CREAMED_STRAWBERRY:
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Chocolate);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::HotPink);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Bisque);
		break;
	case ColorTheme::HARD_TO_SEE:
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::MidnightBlue);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Navy);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::DarkBlue);
		break;
	case ColorTheme::BLACK_AND_WHITE:
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::DimGray);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::White);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::PETYA:
		ComputeFieldVariables::SetColorSolved(DirectX::XMVectorSet(0.639f, 0.694f, 0.745f, 1.0f));
		ComputeFieldVariables::SetColorEnabled(DirectX::XMVectorSet(0.980f, 0.984f, 0.988f, 1.0f));
		ComputeFieldVariables::SetColorNone(DirectX::XMVectorSet(0.459f, 0.733f, 0.992f, 1.0f));
		break;
	default:
		break;
	}
}

void LightsOutRenderer::EdgeColorAsUnlit()
{
	ComputeFieldVariables::SetColorBetweenAsNone();
}

void LightsOutRenderer::EdgeColorAsLit()
{
	ComputeFieldVariables::SetColorBetweenAsEnabled();
}

void LightsOutRenderer::EdgeColorAsSolution()
{
	ComputeFieldVariables::SetColorBetweenAsSolved();
}

void LightsOutRenderer::RedrawFieldToTexture(uint16_t cellSize, uint16_t gameSize)
{
	FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	md3dContext->ClearUnorderedAccessViewFloat(LOTextures::getResultUAV(), clearColor);

	//--------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------Draw the field on the off-screen texture------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------

	ComputeFieldVariables::SetHintTurn((USHORT)(-1), (USHORT)(-1)); //Hints are deprecated

	ComputeFieldVariables::UpdateCSCBuffer(md3dContext.Get());
	ComputeFieldVariables::SetCSVariables(md3dContext.Get());

	switch (mDrawType)
	{
	case DrawType::DRAW_SQUARES:
		ComputeFieldShaders::SetComputeFieldShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CIRCLES:
		ComputeFieldShaders::SetComputeFieldCirclesShader(md3dContext.Get());
		break;
	case DrawType::DRAW_RAINDROPS:
		ComputeFieldShaders::SetComputeFieldRaindropsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CHAINS:
		ComputeFieldShaders::SetComputeFieldChainsShader(md3dContext.Get());
		break;
	default:
		ComputeFieldShaders::SetComputeFieldShader(md3dContext.Get());
		break;
	}

	md3dContext->Dispatch((uint32_t)ceilf(gameSize * (cellSize / 16.0f)),
		                  (uint32_t)ceilf(gameSize * (cellSize / 16.0f)),
		                  1);

	ComputeFieldVariables::DisableVariables(md3dContext.Get());
	ShaderBinder::UnbindComputeShader(md3dContext.Get());
}

void LightsOutRenderer::DrawFieldTexOnScreen()
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

void LightsOutRenderer::DrawField(uint16_t cellSize, uint16_t gameSize)
{
	RedrawFieldToTexture(cellSize, gameSize);
	DrawFieldTexOnScreen();

	mSwapChain->Present(0, 0);
}