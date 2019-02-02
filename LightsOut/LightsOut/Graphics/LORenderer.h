#ifndef LIGHTS_OUT_RENDERER_HPP
#define LIGHTS_OUT_RENDERER_HPP

#include <d3d11.h>
#include <wrl\client.h>
#include <memory>
#include <vector>
#include "LOShaders.hpp"
#include "LOShaderVariables.hpp"
#include "LOTextures.hpp"
#include "RenderStates.hpp"
#include "..\Math\LightsOutBoard.hpp"

enum class DrawType
{
	DRAW_SQUARES,
	DRAW_CIRCLES,
	DRAW_DIAMONDS,
	DRAW_RAINDROPS,
	DRAW_CHAINS,
	DRAW_SQUARES_DOMAIN,
	DRAW_CIRCLES_DOMAIN,
	DRAW_DIAMONDS_DOMAIN,
	DRAW_RAINDROPS_DOMAIN,
	DRAW_CHAINS_DOMAIN
};

enum class ColorTheme
{
	RED_EXPLOSION,
	NEON_XXL,
	AUTUMM,
	CREAMED_STRAWBERRY,
	HARD_TO_SEE,
	BLACK_AND_WHITE,
	PETYA
};

/*
* The renderer class for Lights Out game.
*
*/

class LightsOutRenderer
{
public:
	LightsOutRenderer(HWND hwnd);
	~LightsOutRenderer();

	void ResetBoardSize(uint16_t newSize);
	void ResetDomainSize(uint16_t newDomainSize);
	void SetSolutionVisible(bool visible);
	void SetStabilityVisible(bool visible);

	void DrawBoard(uint16_t cellSize, uint16_t gameSize);
	void DrawBgBoardToMemory(uint16_t cellSize, uint16_t gameSize, std::vector<uint32_t>& outData, uint32_t& outRowPitch);
	
	bool OnWndResize(uint16_t newWidth, uint16_t newHeight);

	void SetDrawTypeSquares();
	void SetDrawTypeCircles();
	void SetDrawTypeDiamonds();
	void SetDrawTypeRaindrops();
	void SetDrawTypeChains();

	void SetDrawTypeBinary();
	void SetDrawTypeDomain();

	void SetBoardToDraw(const LightsOutBoard& board);
	void SetSolutionToDraw(const LightsOutBoard& board);
	void SetStabilityToDraw(const LightsOutBoard& board);

	void SetColorTheme(ColorTheme colorTheme);
	void EdgeColorAsUnlit();
	void EdgeColorAsLit();
	void EdgeColorAsSolution();
	void EdgeColorDimmed();

private:
	void DrawBoardOnTexture(uint16_t cellSize, uint16_t gameSize);
	void DrawBoardTexOnScreen();

	bool IsDrawTypeBinary();
	bool IsDrawTypeDomain();

	bool IsDrawTypeSquares();
	bool IsDrawTypeCircles();
	bool IsDrawTypeDiamonds();
	bool IsDrawTypeRaindrops();
	bool IsDrawTypeChains();

private:
	Microsoft::WRL::ComPtr<ID3D11Device>         md3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>  md3dContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>		 mSwapChain;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	D3D11_VIEWPORT			mViewport;

	std::unique_ptr<ComputeBoardShaders>   mComputeBoardShaders;
	std::unique_ptr<DrawScreenShaders>     mDrawScreenShaders;
	std::unique_ptr<ComputeBoardVariables> mComputeBoardVariables;
	std::unique_ptr<DrawScreenVariables>   mDrawScreenVariables;

	std::unique_ptr<LOTextures>   mTextures;
	std::unique_ptr<RenderStates> mRenderStates;

	DrawType mDrawType;
};

#endif LIGHTS_OUT_RENDERER_HPP