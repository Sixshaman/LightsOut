#ifndef LIGHTS_OUT_RENDERER_HPP
#define LIGHTS_OUT_RENDERER_HPP

#include <d3d11.h>
#include <wrl\client.h>
#include <memory>
#include <boost\dynamic_bitset.hpp>

enum class DrawType
{
	DRAW_SQUARES,
	DRAW_CIRCLES,
	DRAW_RAINDROPS,
	DRAW_CHAINS
};

enum class ColorTheme
{
	RED_EXPLOSION,
	NEON_XXL,
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
	LightsOutRenderer();
	~LightsOutRenderer();

	bool InitD3D(HWND hwnd);

	void ResetFieldSize(uint16_t newSize);
	void SetSolutionVisible(bool visible);
	void SetStabilityVisible(bool visible);

	void DrawField(uint16_t cellSize, uint16_t gameSize);
	void DrawBgFieldToMemory(uint16_t cellSize, uint16_t gameSize, std::vector<uint32_t>& outData, uint32_t& outRowPitch);
	
	bool OnWndResize(uint16_t newWidth, uint16_t newHeight);

	void SetDrawType(DrawType drawType);

	void SetFieldBufferData(boost::dynamic_bitset<uint32_t>);
	void SetSolutionBufferData(boost::dynamic_bitset<uint32_t>);
	void SetStabilityBufferData(boost::dynamic_bitset<uint32_t>);

	void SetColorTheme(ColorTheme colorTheme);
	void EdgeColorAsUnlit();
	void EdgeColorAsLit();
	void EdgeColorAsSolution();
	void EdgeColorDimmed();

private:
	void DrawFieldOnTexture(uint16_t cellSize, uint16_t gameSize);
	void DrawFieldTexOnScreen();

private:
	Microsoft::WRL::ComPtr<ID3D11Device>         md3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>  md3dContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>		 mSwapChain;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	D3D11_VIEWPORT			mViewport;

	DrawType mDrawType;
};

#endif LIGHTS_OUT_RENDERER_HPP