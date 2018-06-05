#include "LightsOut.hpp"
#include "Util.hpp"
#include <WindowsX.h>
#include "LOSaver.hpp"
#include "FileDialog.hpp"

#define IS_RANDOM_SOLVING       0x01 //Call LightsOutSolver::GetRandomTurn() instead of LightsOutSolver::GetFirstTurn()
#define SHOW_SOLUTION           0x02 //Show the whole solution with special color
#define SHOW_STABILITY          0x04 //Show the cell stability data
#define IS_PERIOD_COUNTING      0x08 //Replace the field with the solution each tick
#define IS_EIGVEC_COUNTING      0x10 //Replace the field with the special buffer each tick. After enough ticks you get the eigenvector of the field
#define IS_PERIO4_COUNTING      0x20 //Replace the field with the solution of the solution of the solution of the solution each tick
#define IS_PERIOD_BACK_COUNTING 0x40 //Replace the field with the anti-solution each tick

#define MENU_THEME_RED_EXPLOSION	  1001
#define MENU_THEME_NEON_XXL			  1002
#define MENU_THEME_CREAMED_STRAWBERRY 1003
#define MENU_THEME_HARD_TO_SEE	      1004
#define MENU_THEME_BLACK_AND_WHITE    1005
#define MENU_THEME_PETYA              1006

#define MENU_THEME_EDGES_LIKE_OFF     1201
#define MENU_THEME_EDGES_LIKE_ON      1202
#define MENU_THEME_EDGES_LIKE_SOLVED  1203
#define MENU_THEME_EDGES_DIMMED       1204

#define MENU_VIEW_SQUARES			  2001
#define MENU_VIEW_CIRCLES			  2002
#define MENU_VIEW_RAINDROPS			  2003
#define MENU_VIEW_CHAINS			  2004

#define MENU_VIEW_NO_EDGES			  2100

#define MENU_FILE_SAVE_STATE		  3001
#define MENU_FILE_SAVE_STATE_4X		  3002
#define MENU_FILE_SAVE_STATE_16X	  3003
#define MENU_FILE_SAVE_STATE_05X	  3004
#define MENU_FILE_SAVE_STATE_01X	  3005

namespace
{
	LightsOutApp *gApp = nullptr;
};

LightsOutApp::LightsOutApp(HINSTANCE hInstance): mAppInst(hInstance), mMainWnd(nullptr), mFlags(0), mWndWidth(0), 
												 mWndHeight(0), mSolver(), mPeriodCount(0), mCurrentTurn(-1, -1)
{
	mCellSize = (uint32_t)(ceilf(EXPECTED_WND_SIZE / mGame.getSize()) - 1);

	gApp = this;
}

LightsOutApp::~LightsOutApp()
{
	DestroyMenu(mMainMenu);
}

bool LightsOutApp::InitAll()
{
	if(!InitWnd())
	{
		return false;
	}

	if(!InitMenu())
	{
		return false;
	}

	if(!mRenderer.InitD3D(mMainWnd))
	{
		return false;
	}

	ChangeGameSize(15);

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
	wclass.lpfnWndProc = [](HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam) { return gApp->AppProc(hwnd, message, wparam, lparam); };
	wclass.lpszClassName = L"LightsOutWindow";
	wclass.lpszMenuName = L"LightsOutMenu";
	wclass.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClass(&wclass))
	{
		MessageBox(NULL, L"The main window class cannot be created!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	DWORD wndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	uint32_t wndSize = mGame.getSize() * mCellSize + 1;

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

	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuTheme, L"&Theme");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuView,  L"&View");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuFile,  L"&File");

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_RED_EXPLOSION,		L"Red explosion");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_NEON_XXL,			L"Neon XXL");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_CREAMED_STRAWBERRY, L"Creamed strawberry");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_HARD_TO_SEE,	    L"Hard to see");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_BLACK_AND_WHITE,	L"Black and white");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_PETYA,              L"Petya");
	AppendMenu(MenuTheme, MF_MENUBREAK, 0, nullptr);

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_OFF,	   L"Edges like unlit");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_ON,	   L"Edges like lit");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_SOLVED, L"Edges like solution");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_DIMMED,      L"Dimmed edges");

	AppendMenu(MenuView, MF_STRING, MENU_VIEW_SQUARES,   L"Squares");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_CIRCLES,   L"Circles");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_RAINDROPS, L"Raindrops");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_CHAINS,    L"Chains");
	AppendMenu(MenuView, MF_MENUBREAK, 0, nullptr);

	AppendMenu(MenuView, MF_STRING, MENU_VIEW_NO_EDGES, L"Disable edges");

	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE,     L"Save state 1x   size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_4X,  L"Save state 4x   size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_16X, L"Save state 16x  size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_05X, L"Save state 0.5x size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_01X, L"Save state 0.1x size...");

	SetMenu(mMainWnd, mMainMenu);

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
			mRenderer.DrawField(mCellSize, mGame.getSize());
		}
	}

	return msg.wParam;
}

void LightsOutApp::OnMenuItem(WPARAM State)
{
	switch(LOWORD(State))
	{
	case MENU_THEME_RED_EXPLOSION:
	{
		mRenderer.SetColorTheme(ColorTheme::RED_EXPLOSION);
		break;
	}
	case MENU_THEME_NEON_XXL:
	{
		mRenderer.SetColorTheme(ColorTheme::NEON_XXL);
		break;
	}
	case MENU_THEME_CREAMED_STRAWBERRY:
	{
		mRenderer.SetColorTheme(ColorTheme::CREAMED_STRAWBERRY);
		break;
	}
	case MENU_THEME_HARD_TO_SEE:
	{
		mRenderer.SetColorTheme(ColorTheme::HARD_TO_SEE);
		break;
	}
	case MENU_THEME_BLACK_AND_WHITE:
	{
		mRenderer.SetColorTheme(ColorTheme::BLACK_AND_WHITE);
		break;
	}
	case MENU_THEME_PETYA:
	{
		mRenderer.SetColorTheme(ColorTheme::PETYA);
		break;
	}
	case MENU_THEME_EDGES_LIKE_OFF:
	{
		mRenderer.EdgeColorAsUnlit();
		break;
	}
	case MENU_THEME_EDGES_LIKE_ON:
	{
		mRenderer.EdgeColorAsLit();
		break;
	}
	case MENU_THEME_EDGES_LIKE_SOLVED:
	{
		mRenderer.EdgeColorAsSolution();
		break;
	}
	case MENU_THEME_EDGES_DIMMED:
	{
		mRenderer.EdgeColorDimmed();
		break;
	}
	case MENU_VIEW_SQUARES:
	{
		mRenderer.SetDrawType(DrawType::DRAW_SQUARES);
		break;
	}
	case MENU_VIEW_CIRCLES:
	{
		mRenderer.SetDrawType(DrawType::DRAW_CIRCLES);
		break;
	}
	case MENU_VIEW_RAINDROPS:
	{
		mRenderer.SetDrawType(DrawType::DRAW_RAINDROPS);
		break;
	}
	case MENU_VIEW_CHAINS:
	{
		mRenderer.SetDrawType(DrawType::DRAW_CHAINS);
		break;
	}
	case MENU_VIEW_NO_EDGES:
	{
		break;
	}
	case MENU_FILE_SAVE_STATE:
	{
		SaveField(EXPECTED_WND_SIZE);
		break;
	}
	case MENU_FILE_SAVE_STATE_4X:
	{
		SaveField(EXPECTED_WND_SIZE * 2);
		break;
	}
	case MENU_FILE_SAVE_STATE_16X:
	{
		SaveField(EXPECTED_WND_SIZE * 4);
		break;
	}
	case MENU_FILE_SAVE_STATE_05X:
	{
		SaveField(EXPECTED_WND_SIZE / 2);
		break;
	}
	case MENU_FILE_SAVE_STATE_01X:
	{
		SaveField(EXPECTED_WND_SIZE / 10);
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
			PointOnField turn = mSolver.GetRandomTurn();
			mGame.Click(turn.field_X, turn.field_Y);
		}
		else
		{
			PointOnField turn = mSolver.GetFirstTurn();
			mGame.Click(turn.field_X, turn.field_Y);
		}

		mRenderer.SetFieldBufferData(mGame.getField());
	}

	if((mFlags & IS_PERIOD_COUNTING) == 0 && (mFlags & IS_PERIOD_BACK_COUNTING) == 0 && mPeriodCount != 0) //Special state: no period flag is set, but period isn't zero. That means we've counted the period in the previous tick and now we should clean everything
	{
		mCountedField.clear();
		MessageBox(nullptr, (L"Solution period is " + std::to_wstring(mPeriodCount)).c_str(), L"Soluion period", MB_OK);

		mPeriodCount = 0;
	}

	//Soiution period is being counted, redraw the field with the derived field.
	//Stop if we reached the first field.
	if(mFlags & IS_PERIOD_COUNTING)
	{
		mPeriodCount++;

		boost::dynamic_bitset<uint32_t> solution = mSolver.GetSolution(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &solution);

		mRenderer.SetFieldBufferData(mGame.getField());
		if (mFlags & SHOW_STABILITY)
		{
			mRenderer.SetStabilityBufferData(mGame.getStability());
		}

		if (mCountedField == solution)
		{
			mFlags &= ~IS_PERIOD_COUNTING; //Next tick we'll show the messagebox
		}
	}

	//Soiution period is being counted backwards, redraw the field with the derived field.
	//Stop if we reached the first field.
	if(mFlags & IS_PERIOD_BACK_COUNTING)
	{
		mPeriodCount++;

		boost::dynamic_bitset<uint32_t> invsolution = mSolver.GetInverseSolution(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &invsolution);

		mRenderer.SetFieldBufferData(mGame.getField());
		if (mFlags & SHOW_STABILITY)
		{
			mRenderer.SetStabilityBufferData(mGame.getStability());
		}

		if (mCountedField == invsolution)
		{
			mFlags &= ~IS_PERIOD_BACK_COUNTING; //Next tick we'll show the messagebox
		}
	}

	if (mFlags & IS_PERIO4_COUNTING)
	{
		boost::dynamic_bitset<uint32_t> solution = mSolver.GetSolution(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &solution);

		solution = mSolver.GetSolution(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &solution);

		solution = mSolver.GetSolution(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &solution);

		solution = mSolver.GetSolution(mGame);
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &solution);

		mRenderer.SetFieldBufferData(mGame.getField());
	}

	if(mFlags & IS_EIGVEC_COUNTING)
	{
		mGame.Click(mCurrentTurn.field_X, mCurrentTurn.field_Y);

		boost::dynamic_bitset<uint32_t> resolvent = mSolver.GetSolution(mGame);
		if(mGame.getField() == resolvent)
		{
			mFlags &= ~IS_EIGVEC_COUNTING;
		}

		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &resolvent);
		mRenderer.SetFieldBufferData(mGame.getField());
	}
}

void LightsOutApp::SaveField(uint32_t expectedSize)
{
	uint32_t cellSize = (uint32_t)(ceilf(expectedSize / mGame.getSize()) - 1);

	std::vector<uint32_t> fieldData;
	uint32_t fieldTexRowPitch;
	mRenderer.DrawBgFieldToMemory(cellSize, mGame.getSize(), fieldData, fieldTexRowPitch);

	std::wstring filePath;
	FileDialog::GetPictureToSave(mMainWnd, filePath);

	uint32_t texSize = mGame.getSize() * cellSize + 1;

	LightsOutSaver::SaveBMP(filePath, &fieldData[0], texSize, texSize, fieldTexRowPitch);

	mRenderer.ResetFieldSize(mGame.getSize());
}

void LightsOutApp::OnMouseClick(WPARAM btnState, uint32_t xPos, uint32_t yPos)
{
	uint32_t wndSize = mGame.getSize() * mCellSize + 1;

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

	mRenderer.SetFieldBufferData(mGame.getField());

	if(mFlags & SHOW_SOLUTION)
	{
		mSolution = mSolver.GetSolution(mGame);
		mRenderer.SetSolutionBufferData(mSolution);
	}
}

void LightsOutApp::ChangeGameSize(unsigned short newSize)
{
	mFlags &= ~SHOW_SOLUTION;
	mFlags &= ~IS_PERIOD_COUNTING;
	mFlags &= ~IS_PERIO4_COUNTING;
	mFlags &= ~IS_EIGVEC_COUNTING;
	mPeriodCount = 0;
	mCurrentTurn = PointOnField(-1, -1);
	mCountedField.clear();

	Clamp(newSize, (unsigned short)MINIMUM_FIELD_SIZE, (unsigned short)MAXIMUM_FIELD_SIZE);
	mGame.ResetField(newSize);
	
	mRenderer.ResetFieldSize(newSize);

	wchar_t title[50];
	swprintf_s(title, L"Lights out %dx%d", newSize, newSize);
	SetWindowText(mMainWnd, title);

	mCellSize = (uint32_t)(ceilf(EXPECTED_WND_SIZE / newSize) - 1);
	uint32_t newWndSize = newSize * mCellSize + 1;

	RECT WndRect;
	GetWindowRect(mMainWnd, &WndRect);
	uint32_t WindowPosX = WndRect.left;
	uint32_t WindowPosY = WndRect.top;

	DWORD wndStyle = GetWindowStyle(mMainWnd);

	RECT R = {0, 0, (LONG)newWndSize, (LONG)newWndSize};
	AdjustWindowRect(&R, wndStyle, TRUE);
	mWndWidth  = R.right - R.left;
	mWndHeight = R.bottom - R.top;

	SetWindowPos(mMainWnd, HWND_NOTOPMOST, WindowPosX, WindowPosY, mWndWidth, mWndHeight, 0);
	mRenderer.OnWndResize(mWndWidth, mWndHeight);

	mRenderer.SetFieldBufferData(mGame.getField());
}

void LightsOutApp::ResetField(WPARAM key)
{
	switch(key)
	{
	case VK_OEM_PLUS:
	{
		mFlags &= ~SHOW_STABILITY;
		ChangeGameSize(mGame.getSize() + 1);
		break;
	}
	case VK_OEM_MINUS:
	{
		mFlags &= ~SHOW_STABILITY;
		ChangeGameSize(mGame.getSize() - 1);
		break;
	}
	case 'S':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mFlags |= IS_RANDOM_SOLVING;
		mSolver.SolveGame(mGame);
		break;
	}
	case 'C':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mFlags &= ~IS_RANDOM_SOLVING;
		mSolver.SolveGame(mGame);
		break;
	}
	case 'R':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_SOLVABLE_RANDOM);
		mGame.ResetStability();
		break;
	}
	case 'F':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_FULL_RANDOM);
		mGame.ResetStability();
		break;
	}
	case '0':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_ZERO_ELEMENT);
		mGame.ResetStability();
		break;
	}
	case '1':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_ONE_ELEMENT);
		mGame.ResetStability();
		break;
	}
	case 'B':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_BLATNOY);
		mGame.ResetStability();
		break;
	}
	case 'A':
	{
		mFlags &= ~SHOW_SOLUTION;
		mFlags ^= SHOW_STABILITY;
		if (mFlags & SHOW_STABILITY)
		{
			auto stability = mGame.getStability();
			mRenderer.SetStabilityBufferData(stability);
		}
		break;
	}
	case 'P':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_PETYA_STYLE);
		mGame.ResetStability();
		break;
	}
	case 'O':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;
		mGame.ResetField(mGame.getSize(), RESET_BORDER);
		mGame.ResetStability();
		break;
	}
	case 'E':
	{
		if(mFlags & SHOW_SOLUTION)
		{
			mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &mSolution);
			mRenderer.SetStabilityBufferData(mGame.getStability());
			mFlags &= ~SHOW_SOLUTION;
		}
		else if (mFlags & SHOW_STABILITY)
		{
			mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &mGame.getStability());
			mGame.ResetStability();
			mRenderer.SetStabilityBufferData(mGame.getStability());
			mFlags &= ~SHOW_STABILITY;
		}

		break;
	}
	case 'T':
	{
		mFlags ^= SHOW_SOLUTION;
		
		if(mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame);
			mRenderer.SetSolutionBufferData(mSolution);
		}
		else
		{
			mSolution.clear();
		}

		break;
	}
	case 'W':
	{
		mFlags ^= SHOW_SOLUTION;

		if (mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetInverseSolution(mGame);
			mRenderer.SetSolutionBufferData(mSolution);
		}
		else
		{
			mSolution.clear();
		}

		break;
	}
	case 'I':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;

		boost::dynamic_bitset<uint32_t> invertSolution = ~(mGame.getField());
		mGame.ResetField(mGame.getSize(), RESET_RESOLVENT, &invertSolution);
		mGame.ResetStability();
		break;
	}
	case 'V':
	{
		mFlags &= ~IS_PERIO4_COUNTING;
		mFlags &= ~IS_PERIOD_BACK_COUNTING;

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
		mFlags &= ~IS_PERIOD_BACK_COUNTING;

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
	case 'Z':
	{
		mFlags &= ~IS_PERIOD_COUNTING;
		mFlags &= ~IS_PERIO4_COUNTING;

		if (!(mFlags & IS_PERIOD_BACK_COUNTING))
		{
			mFlags |= IS_PERIOD_BACK_COUNTING;
			mCountedField = mGame.getField();
		}
		else
		{
			mFlags &= ~IS_PERIOD_BACK_COUNTING;
			mCountedField.clear();
		}

		mPeriodCount = 0;

		break;
	}
	}

	mRenderer.SetSolutionVisible((mFlags & SHOW_SOLUTION) != 0);
	mRenderer.SetStabilityVisible(((mFlags & SHOW_SOLUTION) == 0) && (mFlags & SHOW_STABILITY) != 0);
	mRenderer.SetFieldBufferData(mGame.getField());
}

LRESULT CALLBACK LightsOutApp::AppProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
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

