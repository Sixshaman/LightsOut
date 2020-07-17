#include "LORenderer.h"
#include "RenderStates.hpp"
#include <DirectXColors.h>
#include "LOTextures.hpp"
#include "..\Util.hpp"

LightsOutRenderer::LightsOutRenderer(HWND hwnd): mDrawType(DrawType::DRAW_SQUARES)
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
	scDesc.BufferCount = 2;
	scDesc.OutputWindow = hwnd;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;

	ThrowIfFailed(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
		                                        &scDesc, &mSwapChain, &md3dDevice, &featurelvl, &md3dContext));

	if (featurelvl < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(nullptr, L"D3D11 is not supported!", L"Error", MB_ICONERROR | MB_OK);
	}

	OnWndResize(wndWidth, wndHeight);

	mDrawScreenShaders   = std::make_unique<DrawScreenShaders>(md3dDevice.Get());
	mComputeBoardShaders = std::make_unique<ComputeBoardShaders>(md3dDevice.Get());

	mComputeBoardVariables = std::make_unique<ComputeBoardVariables>(md3dDevice.Get());
	mDrawScreenVariables   = std::make_unique<DrawScreenVariables>(md3dDevice.Get());

	mRenderStates = std::make_unique<RenderStates>(md3dDevice.Get());
	mTextures     = std::make_unique<LOTextures>(md3dDevice.Get());

	mDrawScreenVariables->SetSamplerState(mRenderStates->TexturePointSamplerState());
	mDrawScreenVariables->SetBoardTexture(mTextures->ResultSRV());

	SetColorTheme(ColorTheme::NEON_XXL);
	EdgeColorAsUnlit();	
}

LightsOutRenderer::~LightsOutRenderer()
{
}

void LightsOutRenderer::ResetBoardSize(uint16_t newSize)
{
	uint32_t cellSize = (uint32_t)(ceilf(EXPECTED_WND_SIZE / newSize) - 1);
	mComputeBoardVariables->SetCellSize(cellSize);

	mComputeBoardVariables->SetBoardSize(newSize);
	mTextures->ResizeBoard(newSize, cellSize, md3dDevice.Get());
}

void LightsOutRenderer::ResetDomainSize(uint16_t newDomainSize)
{
	mComputeBoardVariables->SetDomainSize(newDomainSize);
}

void LightsOutRenderer::SetSolutionVisible(bool visible)
{
	mComputeBoardVariables->SetSolutionVisible(visible);
}

void LightsOutRenderer::SetStabilityVisible(bool visible)
{
	mComputeBoardVariables->SetStabilityVisible(visible);
}

void LightsOutRenderer::SetToroidRender(bool toroid)
{
	mComputeBoardVariables->SetToroidRender(toroid);
}

bool LightsOutRenderer::OnWndResize(uint16_t newWidth, uint16_t newHeight)
{
	mRenderTarget.Reset();
	mDepthStencilView.Reset();

	ThrowIfFailed(mSwapChain->ResizeBuffers(2, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	ThrowIfFailed(md3dDevice->CreateRenderTargetView(backBuffer.Get(), NULL, mRenderTarget.GetAddressOf()));

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

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	ThrowIfFailed(md3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthStencilBuffer.GetAddressOf()));
	ThrowIfFailed(md3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, mDepthStencilView.GetAddressOf()));

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

void LightsOutRenderer::SetDrawTypeSquares()
{
	if(IsDrawTypeBinary())
	{
		mDrawType = DrawType::DRAW_SQUARES;
	}
	else if(IsDrawTypeDomain())
	{
		mDrawType = DrawType::DRAW_SQUARES_DOMAIN;
	}
}

void LightsOutRenderer::SetDrawTypeCircles()
{
	if(IsDrawTypeBinary())
	{
		mDrawType = DrawType::DRAW_CIRCLES;
	}
	else if (IsDrawTypeDomain())
	{
		mDrawType = DrawType::DRAW_CIRCLES_DOMAIN;
	}
}

void LightsOutRenderer::SetDrawTypeDiamonds()
{
	if(IsDrawTypeBinary())
	{
		mDrawType = DrawType::DRAW_DIAMONDS;
	}
	else if (IsDrawTypeDomain())
	{
		mDrawType = DrawType::DRAW_DIAMONDS_DOMAIN;
	}
}

void LightsOutRenderer::SetDrawTypeBeams()
{
	if(IsDrawTypeBinary())
	{
		mDrawType = DrawType::DRAW_BEAMS;
	}
	else if (IsDrawTypeDomain())
	{
		mDrawType = DrawType::DRAW_BEAMS_DOMAIN;
	}
}

void LightsOutRenderer::SetDrawTypeRaindrops()
{
	if(IsDrawTypeBinary())
	{
		mDrawType = DrawType::DRAW_RAINDROPS;
	}
	else if (IsDrawTypeDomain())
	{
		mDrawType = DrawType::DRAW_RAINDROPS_DOMAIN;
	}
}

void LightsOutRenderer::SetDrawTypeChains()
{
	if(IsDrawTypeBinary())
	{
		mDrawType = DrawType::DRAW_CHAINS;
	}
	else if (IsDrawTypeDomain())
	{
		mDrawType = DrawType::DRAW_CHAINS_DOMAIN;
	}
}

void LightsOutRenderer::SetDrawTypeBinary()
{
	if(IsDrawTypeSquares())
	{
		mDrawType = DrawType::DRAW_SQUARES;
	}
	else if(IsDrawTypeCircles())
	{
		mDrawType = DrawType::DRAW_CIRCLES;
	}
	else if(IsDrawTypeDiamonds())
	{
		mDrawType = DrawType::DRAW_DIAMONDS;
	}
	else if(IsDrawTypeBeams())
	{
		mDrawType = DrawType::DRAW_BEAMS;
	}
	else if(IsDrawTypeRaindrops())
	{
		mDrawType = DrawType::DRAW_RAINDROPS;
	}
	else if(IsDrawTypeChains())
	{
		mDrawType = DrawType::DRAW_CHAINS;
	}
}

void LightsOutRenderer::SetDrawTypeDomain()
{
	if(IsDrawTypeSquares())
	{
		mDrawType = DrawType::DRAW_SQUARES_DOMAIN;
	}
	else if(IsDrawTypeCircles())
	{
		mDrawType = DrawType::DRAW_CIRCLES_DOMAIN;
	}
	else if(IsDrawTypeDiamonds())
	{
		mDrawType = DrawType::DRAW_DIAMONDS_DOMAIN;
	}
	else if(IsDrawTypeBeams())
	{
		mDrawType = DrawType::DRAW_BEAMS_DOMAIN;
	}
	else if(IsDrawTypeRaindrops())
	{
		mDrawType = DrawType::DRAW_RAINDROPS_DOMAIN;
	}
	else if(IsDrawTypeChains())
	{
		mDrawType = DrawType::DRAW_CHAINS_DOMAIN;
	}
}

void LightsOutRenderer::SetBoardToDraw(const LightsOutBoard& board)
{
	mTextures->UpdateBoard(board, md3dContext.Get());
}

void LightsOutRenderer::SetSolutionToDraw(const LightsOutBoard& solution)
{
	mTextures->UpdateSolution(solution, md3dContext.Get());
}

void LightsOutRenderer::SetStabilityToDraw(const LightsOutBoard& stability)
{
	mTextures->UpdateStability(stability, md3dContext.Get());
}

void LightsOutRenderer::SetColorTheme(ColorTheme colorTheme)
{
	switch (colorTheme)
	{
	case ColorTheme::RED_EXPLOSION:
		mComputeBoardVariables->SetColorSolved(DirectX::Colors::Red);
		mComputeBoardVariables->SetColorEnabled(DirectX::Colors::Crimson);
		mComputeBoardVariables->SetColorNone(DirectX::Colors::Salmon);
		break;
	case ColorTheme::NEON_XXL:
		mComputeBoardVariables->SetColorSolved(DirectX::Colors::Blue);
		mComputeBoardVariables->SetColorEnabled(DirectX::Colors::Lime);
		mComputeBoardVariables->SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::AUTUMM:
		mComputeBoardVariables->SetColorSolved(DirectX::Colors::ForestGreen);
		mComputeBoardVariables->SetColorEnabled(DirectX::Colors::Yellow);
		mComputeBoardVariables->SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::CREAMED_STRAWBERRY:
		mComputeBoardVariables->SetColorSolved(DirectX::Colors::Chocolate);
		mComputeBoardVariables->SetColorEnabled(DirectX::Colors::HotPink);
		mComputeBoardVariables->SetColorNone(DirectX::Colors::Bisque);
		break;
	case ColorTheme::HARD_TO_SEE:
		mComputeBoardVariables->SetColorSolved(DirectX::Colors::MidnightBlue);
		mComputeBoardVariables->SetColorEnabled(DirectX::Colors::Navy);
		mComputeBoardVariables->SetColorNone(DirectX::Colors::DarkBlue);
		break;
	case ColorTheme::BLACK_AND_WHITE:
		mComputeBoardVariables->SetColorSolved(DirectX::Colors::DimGray);
		mComputeBoardVariables->SetColorEnabled(DirectX::Colors::White);
		mComputeBoardVariables->SetColorNone(DirectX::Colors::Black);
		break;
	case ColorTheme::PETYA:
		mComputeBoardVariables->SetColorSolved(DirectX::XMVectorSet(0.639f, 0.694f, 0.745f, 1.0f));
		mComputeBoardVariables->SetColorEnabled(DirectX::XMVectorSet(0.980f, 0.984f, 0.988f, 1.0f));
		mComputeBoardVariables->SetColorNone(DirectX::XMVectorSet(0.459f, 0.733f, 0.992f, 1.0f));
		break;
	default:
		break;
	}
}

void LightsOutRenderer::EdgeColorAsUnlit()
{
	mComputeBoardVariables->SetColorBetweenAsNone();
}

void LightsOutRenderer::EdgeColorAsLit()
{
	mComputeBoardVariables->SetColorBetweenAsEnabled();
}

void LightsOutRenderer::EdgeColorAsSolution()
{
	mComputeBoardVariables->SetColorBetweenAsSolved();
}

void LightsOutRenderer::EdgeColorDimmed()
{
	mComputeBoardVariables->SetColorBetweenAsDimmed();
}

void LightsOutRenderer::DrawBoardOnTexture(uint16_t cellSize, uint16_t gameSize)
{
	FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	md3dContext->ClearUnorderedAccessViewFloat(mTextures->ResultUAV(), clearColor);

	//--------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------Draw the board on the off-screen texture------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------

	mComputeBoardVariables->SetBoardSrv(mTextures->BoardSRV());
	mComputeBoardVariables->SetSolutionSrv(mTextures->SolutionSRV());
	mComputeBoardVariables->SetStabilitySrv(mTextures->StabilitySRV());

	mComputeBoardVariables->SetResultUav(mTextures->ResultUAV());

	mComputeBoardVariables->SetCSVariables(md3dContext.Get());

	switch (mDrawType)
	{
	case DrawType::DRAW_SQUARES:
		mComputeBoardShaders->SetComputeBoardShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CIRCLES:
		mComputeBoardShaders->SetComputeBoardCirclesShader(md3dContext.Get());
		break;
	case DrawType::DRAW_DIAMONDS:
		mComputeBoardShaders->SetComputeBoardDiamondsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_BEAMS:
		mComputeBoardShaders->SetComputeBoardBeamsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_RAINDROPS:
		mComputeBoardShaders->SetComputeBoardRaindropsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CHAINS:
		mComputeBoardShaders->SetComputeBoardChainsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_SQUARES_DOMAIN:
		mComputeBoardShaders->SetComputeEveryBoardShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CIRCLES_DOMAIN:
		mComputeBoardShaders->SetComputeEveryBoardCirclesShader(md3dContext.Get());
		break;
	case DrawType::DRAW_DIAMONDS_DOMAIN:
		mComputeBoardShaders->SetComputeEveryBoardDiamondsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_BEAMS_DOMAIN:
		mComputeBoardShaders->SetComputeEveryBoardBeamsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_RAINDROPS_DOMAIN:
		mComputeBoardShaders->SetComputeEveryBoardRaindropsShader(md3dContext.Get());
		break;
	case DrawType::DRAW_CHAINS_DOMAIN:
		mComputeBoardShaders->SetComputeEveryBoardChainsShader(md3dContext.Get());
		break;
	default:
		mComputeBoardShaders->SetComputeBoardShader(md3dContext.Get());
		break;
	}

	md3dContext->Dispatch((uint32_t)ceilf(gameSize * (cellSize / 16.0f)),
		                  (uint32_t)ceilf(gameSize * (cellSize / 16.0f)),
		                  1);

	mComputeBoardVariables->DisableVariables(md3dContext.Get());
	mComputeBoardShaders->UnbindComputeShader(md3dContext.Get());
}

void LightsOutRenderer::DrawBoardTexOnScreen()
{
	ID3D11RenderTargetView* rtvs[] = {mRenderTarget.Get()};
	md3dContext->OMSetRenderTargets(1, rtvs, mDepthStencilView.Get());

	FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	md3dContext->ClearRenderTargetView(mRenderTarget.Get(), clearColor);
	md3dContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	md3dContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);

	md3dContext->IASetInputLayout(nullptr);

	md3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	mDrawScreenVariables->SetBoardTexture(mTextures->ResultSRV());
	mDrawScreenVariables->SetSamplerState(mRenderStates->TexturePointSamplerState());

	mDrawScreenVariables->SetAllVariables(md3dContext.Get());
	mDrawScreenShaders->SetPipelineShaders(md3dContext.Get());

	md3dContext->Draw(4, 0);

	mDrawScreenVariables->DisableVariables(md3dContext.Get());
	mDrawScreenShaders->UnbindPipelineShaders(md3dContext.Get());
}

bool LightsOutRenderer::IsDrawTypeBinary()
{
	return (mDrawType == DrawType::DRAW_SQUARES) || (mDrawType == DrawType::DRAW_CIRCLES) || (mDrawType == DrawType::DRAW_DIAMONDS) || (mDrawType == DrawType::DRAW_BEAMS) || (mDrawType == DrawType::DRAW_RAINDROPS) || (mDrawType == DrawType::DRAW_CHAINS);
}

bool LightsOutRenderer::IsDrawTypeDomain()
{
	return (mDrawType == DrawType::DRAW_SQUARES_DOMAIN) || (mDrawType == DrawType::DRAW_CIRCLES_DOMAIN) || (mDrawType == DrawType::DRAW_DIAMONDS_DOMAIN) || (mDrawType == DrawType::DRAW_BEAMS_DOMAIN) || (mDrawType == DrawType::DRAW_RAINDROPS_DOMAIN) || (mDrawType == DrawType::DRAW_CHAINS_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeSquares()
{
	return (mDrawType == DrawType::DRAW_SQUARES) || (mDrawType == DrawType::DRAW_SQUARES_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeCircles()
{
	return (mDrawType == DrawType::DRAW_CIRCLES) || (mDrawType == DrawType::DRAW_CIRCLES_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeDiamonds()
{
	return (mDrawType == DrawType::DRAW_DIAMONDS) || (mDrawType == DrawType::DRAW_DIAMONDS_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeBeams()
{
	return (mDrawType == DrawType::DRAW_BEAMS) || (mDrawType == DrawType::DRAW_BEAMS_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeRaindrops()
{
	return (mDrawType == DrawType::DRAW_RAINDROPS) || (mDrawType == DrawType::DRAW_RAINDROPS_DOMAIN);
}

bool LightsOutRenderer::IsDrawTypeChains()
{
	return (mDrawType == DrawType::DRAW_CHAINS) || (mDrawType == DrawType::DRAW_CHAINS_DOMAIN);
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

	mComputeBoardVariables->SetCellSize(cellSize);

	mTextures->ResizeBoard(gameSize, cellSize, md3dDevice.Get());
	DrawBoardOnTexture(cellSize, gameSize);

	ID3D11Texture2D* resultTex = mTextures->MappedTex(md3dContext.Get());

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