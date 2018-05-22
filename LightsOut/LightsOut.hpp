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