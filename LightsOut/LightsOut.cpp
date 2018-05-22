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

#include "LightsOut.hpp"
#include "Util.hpp"
#include "RenderStates.hpp"
#include <WindowsX.h>
#include <DirectXColors.h>
#include "LOSaver.hpp"

#define IS_RANDOM_SOLVING  0x01 //If this flag is set, we call the GetRandomTurn() from mSolver. We call the GetFirstTurn() otherwise.
#define SHOW_RESOLVENT     0x02 //If this flag is set, we show the whole solution with special color
#define IS_PERIOD_COUNTING 0x04 //Bycicly, but cool thread realization
#define IS_EIGVEC_COUNTING 0x08 //Bycicly, but cool thread realization #2
#define IS_PERIO4_COUNTING 0x10 //Bycicly, but cool thread realization #3

#define MENU_THEME_MOTHER_RUSSIA	  1001
#define MENU_THEME_RED_EXPLOSION	  1002
#define MENU_THEME_NEON_XXL			  1003
#define MENU_THEME_CREAMED_STRAWBERRY 1004
#define MENU_THEME_HARD_TO_SEE	      1005
#define MENU_THEME_BLACK_AND_WHITE    1006

#define MENU_THEME_BLACK_EDGES		  1100
#define MENU_THEME_GRAY_EDGES		  1101
#define MENU_THEME_WHITE_EDGES		  1102

#define MENU_THEME_EDGES_LIKE_OFF     1200
#define MENU_THEME_EDGES_LIKE_ON      1201
#define MENU_THEME_EDGES_LIKE_SOLVED  1202

#define MENU_VIEW_SQUARES			  2001
#define MENU_VIEW_CIRCLES			  2002
#define MENU_VIEW_RAINDROPS			  2003
#define MENU_VIEW_CHAINS			  2004

#define MENU_FILE_SAVE_STATE		  3001

namespace
{
	LightsOutApp *gApp = nullptr;
};

LightsOutApp::LightsOutApp(HINSTANCE hInstance): mAppInst(hInstance), mMainWnd(nullptr), md3dDevice(nullptr), md3dContext(nullptr), 
												 mSwapChain(nullptr), mRenderTarget(nullptr), mGame(15), mFlags(0), mWndWidth(0), 
												 mWndHeight(0), mSolver(), mHintTurn(nullptr), mScreenQuad(nullptr),
												 mDepthStencilView(nullptr), mPeriodCount(0), mCurrentTurn(-1, -1)
{
	mCellSize = (UINT)(ceilf(EXPECTED_WND_SIZE / mGame.getSize()) - 1);

	mDrawType = DrawType::DRAW_SQUARES;

	gApp = this;
}

LightsOutApp::~LightsOutApp()
{
	DestroyMenu(mMainMenu);

	DrawScreenShaders::DestroyAll();
	DrawScreenVariables::DestroyAll();
	ComputeFieldShaders::DestroyAll();
	ComputeFieldVariables::DestroyAll();
	RenderStates::DestroyAll();
	LOTextures::DestroyAll();

	SafeRelease(md3dDevice);
	SafeRelease(md3dContext);
	SafeRelease(mSwapChain);

	SafeRelease(mRenderTarget);
	SafeRelease(mDepthStencilView);

	SafeDelete(mScreenQuad);

	SafeDelete(mHintTurn);
}

bool LightsOutApp::InitAll()
{
	mScreenQuad = new LOScreenQuad();

	if(!InitWnd())
	{
		return false;
	}

	if(!InitMenu())
	{
		return false;
	}

	if(!InitD3D())
	{
		return false;
	}

	if(!DrawScreenShaders::InitAll(md3dDevice))
	{
		return false;
	}

	if(!ComputeFieldShaders::InitAll(md3dDevice))
	{
		return false;
	}

	if(!DrawScreenVariables::InitAll(md3dDevice))
	{
		return false;
	}

	if(!ComputeFieldVariables::InitAll(md3dDevice))
	{
		return false;
	}

	if(!RenderStates::InitAll(md3dDevice))
	{
		return false;
	}

	if(!LOTextures::InitSRVs(mGame.getField(), mSolver.GetResolvent(mGame), mGame.getSize(), md3dDevice))
	{
		return false;
	}

	if(!mScreenQuad->InitAll(md3dDevice))
	{
		return false;
	}

	ComputeFieldVariables::SetColorSolved(DirectX::Colors::Blue);
	ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Lime);
	ComputeFieldVariables::SetColorNone(DirectX::Colors::Black);
	ComputeFieldVariables::SetColorBetween(DirectX::Colors::DarkGray);

	return true;
}

bool LightsOutApp::InitWnd()
{
	WNDCLASS wclass;

	wclass.cbClsExtra = 0;
	wclass.cbWndExtra = 0;
	wclass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wclass.hInstance = mAppInst;
	wclass.lpfnWndProc = [](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) { return gApp->AppProc(hwnd, message, wparam, lparam); };
	wclass.lpszClassName = L"LightsOutWindow";
	wclass.lpszMenuName = L"LightsOutMenu";
	wclass.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClass(&wclass))
	{
		MessageBox(NULL, L"The main window class cannot be created!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	UINT wndSize = mGame.getSize() * mCellSize + 1;

	RECT R = {0, 0, (LONG)wndSize, (LONG)wndSize};
	AdjustWindowRect(&R, wndStyle, TRUE);
	mWndWidth = R.right - R.left;
	mWndHeight = R.bottom - R.top;
	
	mMainWnd = CreateWindow(L"LightsOutWindow", L"Lights out 15x15", wndStyle, 0, 0,
							mWndWidth, mWndHeight, nullptr, nullptr, mAppInst, 0);

	if(!mMainWnd)
	{
		MessageBox(NULL, L"The main window cannot be created!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	ShowWindow(mMainWnd, SW_SHOWDEFAULT);
	UpdateWindow(mMainWnd);

	return true;
}

bool LightsOutApp::InitMenu()
{
	mMainMenu = CreateMenu();

	HMENU MenuTheme = CreatePopupMenu();
	HMENU MenuView = CreatePopupMenu();
	HMENU MenuFile = CreatePopupMenu();

	if(!mMainMenu || !MenuTheme || !MenuFile)
	{
		return false;
	}

	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (UINT)MenuTheme, L"&Theme");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (UINT)MenuView,  L"&View");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (UINT)MenuFile,  L"&File");

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_MOTHER_RUSSIA,		L"Mother Russia");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_RED_EXPLOSION,		L"Red explosion");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_NEON_XXL,			L"Neon XXL");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_CREAMED_STRAWBERRY, L"Creamed strawberry");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_HARD_TO_SEE,	    L"Hard to see");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_BLACK_AND_WHITE,	L"Black and white");
	AppendMenu(MenuTheme, MF_MENUBREAK, 0, nullptr);

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_BLACK_EDGES, L"Set black edges");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_GRAY_EDGES,  L"Set gray edges");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_WHITE_EDGES, L"Set white edges");
	AppendMenu(MenuTheme, MF_MENUBREAK, 0, nullptr);

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_OFF,	   L"Set edges like off");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_ON,	   L"Set edges like on");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_SOLVED, L"Set edges like solve");

	AppendMenu(MenuView, MF_STRING, MENU_VIEW_SQUARES,   L"Squares");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_CIRCLES,   L"Circles");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_RAINDROPS, L"Raindrops");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_CHAINS,    L"Chains");

	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE, L"Save state...");

	SetMenu(mMainWnd, mMainMenu);

	return true;
}

bool LightsOutApp::InitD3D()
{
	UINT flags = 0;
	D3D_FEATURE_LEVEL featurelvl;

#if defined(DEBUG) || defined(__DEBUG)
	flags | D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.Width = mWndWidth;
	scDesc.BufferDesc.Height = mWndHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 1;
	scDesc.OutputWindow = mMainWnd;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = 0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
											   &scDesc, &mSwapChain, &md3dDevice, &featurelvl, &md3dContext);

	if(FAILED(hr))
	{
		MessageBox(nullptr, L"D3D11 device creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if(featurelvl < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(nullptr, L"D3D11 is not supported!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if(!OnWndResize())
	{
		return false;
	}

	return true;
}

bool LightsOutApp::OnWndResize()
{
	SafeRelease(mRenderTarget);
	SafeRelease(mDepthStencilView);

	if(FAILED(mSwapChain->ResizeBuffers(1, mWndWidth, mWndHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
	{
		MessageBox(nullptr, L"Buffers resizing error", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	ID3D11Texture2D *backBuffer = nullptr;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

	if(FAILED(md3dDevice->CreateRenderTargetView(backBuffer, NULL, &mRenderTarget)))
	{
		MessageBox(nullptr, L"Render target creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	SafeRelease(backBuffer);

	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	depthStencilBufferDesc.Width = mWndWidth;
	depthStencilBufferDesc.Height = mWndHeight;
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
	if(FAILED(md3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &depthStencilBuffer)))
	{
		MessageBox(nullptr, L"Depth buffer creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}
	if(FAILED(md3dDevice->CreateDepthStencilView(depthStencilBuffer, nullptr, &mDepthStencilView)))
	{
		MessageBox(nullptr, L"Depth/stencil view creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	SafeRelease(depthStencilBuffer);

	md3dContext->OMSetRenderTargets(1, &mRenderTarget, mDepthStencilView);

	mViewport.Width = (float)mWndWidth;
	mViewport.Height = (float)mWndHeight;
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	md3dContext->RSSetViewports(1, &mViewport);

	return true;
}

int LightsOutApp::RunApp()
{
	MSG msg = {0};

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();
			DrawField();
		}
	}

	return msg.wParam;
}

void LightsOutApp::OnMenuItem(WPARAM State)
{
	switch(LOWORD(State))
	{
	case MENU_THEME_MOTHER_RUSSIA:
	{
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Red);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Blue);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::White);
		break;
	}
	case MENU_THEME_RED_EXPLOSION:
	{
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Red);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Crimson);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Salmon);
		break;
	}
	case MENU_THEME_NEON_XXL:
	{
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Blue);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Lime);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Black);
		break;
	}
	case MENU_THEME_CREAMED_STRAWBERRY:
	{
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::Chocolate);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::HotPink);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Bisque);
		break;
	}
	case MENU_THEME_HARD_TO_SEE:
	{
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::MidnightBlue);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::Navy);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::DarkBlue);
		break;
	}
	case MENU_THEME_BLACK_AND_WHITE:
	{
		ComputeFieldVariables::SetColorSolved(DirectX::Colors::DimGray);
		ComputeFieldVariables::SetColorEnabled(DirectX::Colors::White);
		ComputeFieldVariables::SetColorNone(DirectX::Colors::Black);
		break;
	}
	case MENU_THEME_BLACK_EDGES:
	{
		ComputeFieldVariables::SetColorBetween(DirectX::Colors::Black);
		break;
	}
	case MENU_THEME_GRAY_EDGES:
	{
		ComputeFieldVariables::SetColorBetween(DirectX::Colors::DarkGray);
		break;
	}
	case MENU_THEME_WHITE_EDGES:
	{
		ComputeFieldVariables::SetColorBetween(DirectX::Colors::White);
		break;
	}
	case MENU_THEME_EDGES_LIKE_OFF:
	{
		ComputeFieldVariables::SetColorBetweenAsNone();
		break;
	}
	case MENU_THEME_EDGES_LIKE_ON:
	{
		ComputeFieldVariables::SetColorBetweenAsEnabled();
		break;
	}
	case MENU_THEME_EDGES_LIKE_SOLVED:
	{
		ComputeFieldVariables::SetColorBetweenAsSolved();
		break;
	}
	case MENU_VIEW_SQUARES:
	{
		mDrawType = DrawType::DRAW_SQUARES;
		break;
	}
	case MENU_VIEW_CIRCLES:
	{
		mDrawType = DrawType::DRAW_CIRCLES;
		break;
	}
	case MENU_VIEW_RAINDROPS:
	{
		mDrawType = DrawType::DRAW_RAINDROPS;
		break;
	}
	case MENU_VIEW_CHAINS:
	{
		mDrawType = DrawType::DRAW_CHAINS;
		break;
	}
	case MENU_FILE_SAVE_STATE:
	{
		LightsOutSaver::SaveState(md3dContext, mMainWnd);
		break;
	}
	}
}

void LightsOutApp::Update()
{
	//If we are allowed to take turns from turns pool, do it
	if(mSolver.GetsolvingFlag())
	{
		if(mFlags & IS_RANDOM_SOLVING)
		{
			PointOnField hode = mSolver.GetRandomTurn();
			mGame.Click(hode.field_X, hode.field_Y);
		}
		else
		{
			PointOnField hode = mSolver.GetFirstTurn();
			mGame.Click(hode.field_X, hode.field_Y);
		}

		LOTextures::UpdateField(mGame.getField(), md3dContext);
	}

	//If the period is counting now, redraw the field with derived field.
	//Stop if we reached the start field.
	if(mFlags & IS_PERIOD_COUNTING)
	{
		boost::dynamic_bitset<UINT> resolvent = mSolver.GetResolvent(mGame);
		if(mCountedField != resolvent)
		{
			mPeriodCount++;
			mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);

			LOTextures::UpdateField(mGame.getField(), md3dContext);
		}
		else
		{
			mCountedField.clear();

			WCHAR aaa[100];
			swprintf_s(aaa, L"Resolvent period is %d", mPeriodCount + 1);
			MessageBox(nullptr, aaa, L"Resolvent period", MB_OK);

			mFlags &= ~IS_PERIOD_COUNTING;
			mPeriodCount = 0;
		}	
	}

	if (mFlags & IS_PERIO4_COUNTING)
	{
		boost::dynamic_bitset<UINT> resolvent = mSolver.GetResolvent(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);

		resolvent = mSolver.GetResolvent(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);

		resolvent = mSolver.GetResolvent(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);

		resolvent = mSolver.GetResolvent(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);

		LOTextures::UpdateField(mGame.getField(), md3dContext);
	}

	if(mFlags & IS_EIGVEC_COUNTING)
	{
		mGame.Click(mCurrentTurn.field_X, mCurrentTurn.field_Y);

		boost::dynamic_bitset<UINT> resolvent = mSolver.GetResolvent(mGame);
		if(mGame.getField() == resolvent)
		{
			mFlags &= ~IS_EIGVEC_COUNTING;
		}

		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);
		LOTextures::UpdateField(mGame.getField(), md3dContext);
	}
}

void LightsOutApp::DrawField()
{
	md3dContext->ClearUnorderedAccessViewFloat(LOTextures::getResultUAV(), reinterpret_cast<const float*>(&XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));

	//--------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------Draw the field on the off-screen texture------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------

	ComputeFieldVariables::SetFieldSize(mGame.getSize());
	ComputeFieldVariables::SetCellSize(mCellSize);
	ComputeFieldVariables::SetSolveVisible((mFlags & SHOW_RESOLVENT) != 0);

	if(mHintTurn)
	{
		ComputeFieldVariables::SetHintTurn(mHintTurn->field_X, mHintTurn->field_Y);
	}
	else
	{
		ComputeFieldVariables::SetHintTurn((USHORT)(-1), (USHORT)(-1));
	}

	ComputeFieldVariables::UpdateCSCBuffer(md3dContext);
	ComputeFieldVariables::SetCSVariables(md3dContext);

	switch (mDrawType)
	{
	case DrawType::DRAW_SQUARES:
		ComputeFieldShaders::SetComputeFieldShader(md3dContext);
		break;
	case DrawType::DRAW_CIRCLES:
		ComputeFieldShaders::SetComputeFieldCirclesShader(md3dContext);
		break;
	case DrawType::DRAW_RAINDROPS:
		ComputeFieldShaders::SetComputeFieldRaindropsShader(md3dContext);
		break;
	case DrawType::DRAW_CHAINS:
		ComputeFieldShaders::SetComputeFieldChainsShader(md3dContext);
		break;
	default:
		ComputeFieldShaders::SetComputeFieldShader(md3dContext);
		break;
	}

	md3dContext->Dispatch((UINT)ceilf(mGame.getSize() * (mCellSize / 16.0f)), 
						  (UINT)ceilf(mGame.getSize() * (mCellSize / 16.0f)), 
						  1);

	ComputeFieldVariables::DisableVariables(md3dContext);
	ShaderBinder::UnbindComputeShader(md3dContext);

	//---------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------Draw the off-screen texture on the screen------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------

	md3dContext->ClearRenderTargetView(mRenderTarget, reinterpret_cast<const float*>(&XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));

	md3dContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mScreenQuad->Draw(md3dContext);

	//----------------------------------------------------------------------------------------------

	mSwapChain->Present(0, 0);
}

void LightsOutApp::OnMouseClick(WPARAM btnState, UINT xPos, UINT yPos)
{
	UINT wndSize = mGame.getSize() * mCellSize + 1;

	int stepX = (wndSize+1) / mGame.getSize();
	int stepY = (wndSize+1) / mGame.getSize();

	unsigned short modX = (unsigned short)(xPos / stepX);
	unsigned short modY = (unsigned short)(yPos / stepY);

	if(btnState & MK_CONTROL)
	{
		mGame.ConstructClick(modX, modY);
	}
	else if(btnState & MK_SHIFT)
	{
		if(mFlags & IS_EIGVEC_COUNTING)
		{
			mFlags &= ~IS_EIGVEC_COUNTING;
		}
		else
		{
			mFlags |= IS_EIGVEC_COUNTING;
			mCurrentTurn = PointOnField(modX, modY);
		}
	}
	else
	{
		mGame.Click(modX, modY);
	}

	LOTextures::UpdateField(mGame.getField(), md3dContext);

	if(mFlags & SHOW_RESOLVENT)
	{
		mTurnsResolvent = mSolver.GetResolvent(mGame);

		LOTextures::UpdateSolve(mTurnsResolvent, md3dContext);
	}

	if(mGame.getField().none())
	{ 
		MessageBox(nullptr, L"You win the game!", L"Congratulations!", MB_OK);
	}

	SafeDelete(mHintTurn);
}

void LightsOutApp::ChangeGameSize(unsigned short newSize)
{
	mFlags &= ~SHOW_RESOLVENT;
	mFlags &= ~IS_PERIOD_COUNTING;
	mFlags &= ~IS_PERIO4_COUNTING;
	mFlags &= ~IS_EIGVEC_COUNTING;
	mPeriodCount = 0;
	mCurrentTurn = PointOnField(-1, -1);
	mCountedField.clear();

	Clamp(newSize, (unsigned short)MINIMUM_FIELD_SIZE, (unsigned short)MAXIMUM_FIELD_SIZE);
	mGame.ResetField(newSize);
	SafeDelete(mHintTurn);
	
	LOTextures::ResizeField(newSize, md3dDevice);

	wchar_t title[50];
	swprintf_s(title, L"Lights out %dx%d", newSize, newSize);
	SetWindowText(mMainWnd, title);

	mCellSize = (UINT)(ceilf(EXPECTED_WND_SIZE / newSize) - 1);
	UINT newWndSize = newSize * mCellSize + 1;

	RECT WndRect;
	GetWindowRect(mMainWnd, &WndRect);
	UINT WindowPosX = WndRect.left;
	UINT WindowPosY = WndRect.top;

	DWORD wndStyle = GetWindowStyle(mMainWnd);

	RECT R = {0, 0, (LONG)newWndSize, (LONG)newWndSize};
	AdjustWindowRect(&R, wndStyle, TRUE);
	mWndWidth  = R.right - R.left;
	mWndHeight = R.bottom - R.top;

	SetWindowPos(mMainWnd, HWND_NOTOPMOST, WindowPosX, WindowPosY, mWndWidth, mWndHeight, 0);
	OnWndResize();
}

void LightsOutApp::ResetField(WPARAM key)
{
	switch(key)
	{
	case VK_OEM_PLUS:
	{
		ChangeGameSize(mGame.getSize() + 1);
		break;
	}
	case VK_OEM_MINUS:
	{
		ChangeGameSize(mGame.getSize() - 1);
		break;
	}
	case 'S':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mFlags |= IS_RANDOM_SOLVING;
		mSolver.SolveGame(mGame);
		break;
	}
	case 'C':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mFlags &= ~IS_RANDOM_SOLVING;
		mSolver.SolveGame(mGame);
		break;
	}
	/*case 'L': //DO NOT CALL. DEPRECATED.
	{
		mResolventFlag = false;
		mIsRandomSolving = false;
		mSolver.SolveGame(mGame);
		mSolver.sortTurnsByCoolness(mGame);
		break;
	}*/
	case 'R':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_SOLVABLE_RANDOM);
		break;
	}
	case 'F':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_FULL_RANDOM);
		break;
	}
	case '0':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_ZERO_ELEMENT);
		break;
	}
	case '1':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_ONE_ELEMENT);
		break;
	}
	case 'A':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_CLICK_ALL);
		break;
	}
	case 'B':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_BLATNOY);
		break;
	}
	case 'P':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_PETYA_STYLE);
		break;
	}
	case 'O':
	{
		mFlags &= ~SHOW_RESOLVENT;
		mGame.ResetField(mGame.getSize(), RESET_BORDER);
		break;
	}
	case 'E':
	{
		if(mFlags & SHOW_RESOLVENT)
		{
			mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &mTurnsResolvent);
		}

		mFlags &= ~SHOW_RESOLVENT;

		break;
	}
	case 'H':
	{
		SafeDelete(mHintTurn);
		mHintTurn = new PointOnField(mSolver.GetHint(mGame));
		break;
	}
	case 'T':
	{
		mFlags ^= SHOW_RESOLVENT;
		
		if(mFlags & SHOW_RESOLVENT)
		{
			mTurnsResolvent = mSolver.GetResolvent(mGame);
			LOTextures::UpdateSolve(mTurnsResolvent, md3dContext);
		}
		else
		{
			mTurnsResolvent.clear();
		}

		break;
	}
	case 'W':
	{
		mFlags ^= SHOW_RESOLVENT;

		if (mFlags & SHOW_RESOLVENT)
		{
			mTurnsResolvent = mSolver.GetInverseResolvent(mGame);
			LOTextures::UpdateSolve(mTurnsResolvent, md3dContext);
		}
		else
		{
			mTurnsResolvent.clear();
		}

		break;
	}
	case 'I':
	{
		mFlags &= ~SHOW_RESOLVENT;

		boost::dynamic_bitset<UINT> invertSolution = ~(mGame.getField());
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &invertSolution);
		break;
	}
	case 'V':
	{
		mFlags &= ~IS_PERIO4_COUNTING;

		if(!(mFlags & IS_PERIOD_COUNTING))
		{
			mFlags |= IS_PERIOD_COUNTING;
			mCountedField = mGame.getField();
		}
		else
		{
			mFlags &= ~IS_PERIOD_COUNTING;
			mCountedField.clear();
		}
		mPeriodCount = 0;

		break;
	}
	case 'X':
	{
		mFlags &= ~IS_PERIOD_COUNTING;

		if (!(mFlags & IS_PERIO4_COUNTING))
		{
			mFlags |= IS_PERIO4_COUNTING;
		}
		else
		{
			mFlags &= ~IS_PERIO4_COUNTING;
		}
		break;
	}
	}

	LOTextures::UpdateField(mGame.getField(), md3dContext);
}

LRESULT CALLBACK LightsOutApp::AppProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool MouseHolding = false;

	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		if(!MouseHolding)
		{
			OnMouseClick(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			MouseHolding = true;
		}
		break;
	case WM_LBUTTONUP:
		MouseHolding = false;
		break;
	case WM_COMMAND:
	{
		OnMenuItem(wParam);
		break;
	}
	case WM_KEYUP:
	{
		ResetField(wParam);
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

