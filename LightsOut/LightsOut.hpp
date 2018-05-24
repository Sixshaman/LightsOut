#ifndef LIGHTS_OUT_HPP
#define LIGHTS_OUT_HPP

#include <d3d11.h>
#include <Windows.h>
#include "LightsOutGame.hpp"
#include "LightsOutSolver.hpp"

#include "LOShaders.hpp"
#include "LOTextures.hpp"
#include "LOScreenQuad.hpp"
#include "LOShaderVariables.hpp"

enum class DrawType
{
	DRAW_SQUARES   = 0,
	DRAW_CIRCLES   = 1,
	DRAW_RAINDROPS = 2,
	DRAW_CHAINS    = 3
};

/*
* The main class for Lights Out game.
*
*/

class LightsOutApp
{
public:
	LightsOutApp(HINSTANCE hInstance);
	~LightsOutApp();

	bool InitAll();
	void Reset(UINT size);

	int RunApp();

	LRESULT CALLBACK AppProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void OnMouseClick(WPARAM btnState, UINT xPos, UINT yPos);

	bool InitWnd();
	bool InitMenu();
	bool InitD3D();

	void Update();
	void DrawField();

	bool OnWndResize();

	void OnMenuItem(WPARAM State);
	void ResetField(WPARAM key);

	void ChangeGameSize(unsigned short newSize);

private:
	HINSTANCE mAppInst;
	HWND	  mMainWnd;
	HMENU     mMainMenu;

	ID3D11Device*		 md3dDevice;
	ID3D11DeviceContext* md3dContext;
	IDXGISwapChain*		 mSwapChain;

	ID3D11RenderTargetView* mRenderTarget;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT			mViewport;

	LOScreenQuad *mScreenQuad; //To draw the field with Direct3D

	LightsOutGame mGame;
	Solver mSolver;

	UINT mCellSize;	//Calculated from game size

	PointOnField			    *mHintTurn;		  //Just a hint
	boost::dynamic_bitset<UINT>  mTurnsResolvent; //Whole solution

	UINT mPeriodCount;
	boost::dynamic_bitset<UINT> mCountedField;

	PointOnField mCurrentTurn;

	int mWndWidth;
	int mWndHeight;

	DrawType mDrawType;

	UINT8 mFlags; //Some flags for showing the field
};

#endif LIGHTS_OUT_HPP