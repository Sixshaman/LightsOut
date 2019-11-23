#include "LightsOut.hpp"
#include "..\Util.hpp"
#include <WindowsX.h>
#include "..\Saving\LOSaver.hpp"
#include "..\Saving\FileDialog.hpp"
#include "LightsOutBoardGen.hpp"

#define IS_RANDOM_SOLVING       0x001 //Call LightsOutSolver::GetRandomTurn() instead of LightsOutSolver::GetFirstTurn()
#define SHOW_SOLUTION           0x002 //Show the whole solution with special color
#define SHOW_INVERSE_SOLUTION   0x004 //Show the whole anti-solution with special color
#define SHOW_STABILITY          0x008 //Show the cell stability data
#define SHOW_LIT_STABILITY      0x010 //Show the cell stability data for lit cells
#define IS_PERIOD_COUNTING      0x020 //Replace the board with the solution each tick
#define IS_EIGVEC_COUNTING      0x040 //Replace the board with the special buffer each tick. After enough ticks you get the eigenvector of the board
#define IS_PERIO4_COUNTING      0x080 //Replace the board with the solution of the solution of the solution of the solution each tick
#define IS_PERIOD_BACK_COUNTING 0x100 //Replace the board with the anti-solution each tick
#define DISPLAY_PERIOD_COUNT    0x200 //Display the period count when counting finished/stopped

#define MENU_THEME_RED_EXPLOSION	  1001
#define MENU_THEME_NEON_XXL			  1002
#define MENU_THEME_AUTUMM			  1003
#define MENU_THEME_CREAMED_STRAWBERRY 1004
#define MENU_THEME_HARD_TO_SEE	      1005
#define MENU_THEME_BLACK_AND_WHITE    1006
#define MENU_THEME_PETYA              1007

#define MENU_THEME_EDGES_LIKE_OFF     1201
#define MENU_THEME_EDGES_LIKE_ON      1202
#define MENU_THEME_EDGES_LIKE_SOLVED  1203
#define MENU_THEME_EDGES_DIMMED       1204

#define MENU_VIEW_SQUARES			  2001
#define MENU_VIEW_CIRCLES			  2002
#define MENU_VIEW_DIAMONDS			  2003
#define MENU_VIEW_BEAMS			      2004
#define MENU_VIEW_RAINDROPS			  2005
#define MENU_VIEW_CHAINS			  2006

#define MENU_VIEW_NO_EDGES			  2100

#define MENU_FILE_SAVE_STATE		  3001
#define MENU_FILE_SAVE_STATE_4X		  3002
#define MENU_FILE_SAVE_STATE_16X	  3003
#define MENU_FILE_SAVE_STATE_05X	  3004
#define MENU_FILE_SAVE_STATE_01X	  3005

#define HOTKEY_ID_CLICKMODE_REGULAR 1001
#define HOTKEY_ID_CLICKMODE_TOROID  1002
#define HOTKEY_ID_CLICKMODE_CUSTOM  1003
#define HOTKEY_ID_CLICKMODE_CUSTOR  1004

#define HOTKEY_ID_PERIOD_COUNT      2001
#define HOTKEY_ID_PERIO4_COUNT      2002
#define HOTKEY_ID_PERIOD_BACK_COUNT 2003

#define HOTKEY_ID_DECREASE_DOMAIN_SIZE 3001
#define HOTKEY_ID_INCREASE_DOMAIN_SIZE 3002

#define HOTKEY_ID_ROTATE_NONZERO 4001
#define HOTKEY_ID_STABLE_LIT     4002

namespace
{
	LightsOutApp *gApp = nullptr;
};

LightsOutApp::LightsOutApp(HINSTANCE hInstance): mAppInst(hInstance), mMainWnd(nullptr), mFlags(0), mWndWidth(0), 
												 mWndHeight(0), mSolver(), mPeriodCount(0), mEigenvecTurn(-1, -1),
	                                             mCountedBoard(15, 2), mSavedBoard(15, 2), mSolution(15, 2)
{
	mCellSize = (uint32_t)(ceilf(EXPECTED_WND_SIZE / mGame.GetSize()) - 1);

	mWorkingMode = WorkingMode::LIT_BOARD;
	mWindowTitle = L"";

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

	if(!InitHotkeys())
	{
		return false;
	}

	mRenderer = std::make_unique<LightsOutRenderer>(mMainWnd);

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
	uint32_t wndSize = mGame.GetSize() * mCellSize + 1;

	RECT R = {0, 0, (LONG)wndSize, (LONG)wndSize};
	AdjustWindowRect(&R, wndStyle, TRUE);
	mWndWidth = R.right - R.left;
	mWndHeight = R.bottom - R.top;
	
	mWindowTitle = L"Lights out 15x15";

	mMainWnd = CreateWindow(L"LightsOutWindow", mWindowTitle.c_str(), wndStyle, 0, 0,
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

	HMENU MenuTheme     = CreatePopupMenu();
	HMENU MenuView      = CreatePopupMenu();
	//HMENU MenuClickRule = CreatePopupMenu();
	HMENU MenuFile      = CreatePopupMenu();

	if(!mMainMenu || !MenuTheme || !MenuView || /*!MenuClickRule ||*/ !MenuFile)
	{
		return false;
	}

	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuTheme,     L"&Theme");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuView,      L"&View");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuFile,      L"&File");

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_RED_EXPLOSION,		L"Red explosion");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_NEON_XXL,			L"Neon XXL");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_AUTUMM,             L"AUTUMN");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_CREAMED_STRAWBERRY, L"Creamed strawberry");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_HARD_TO_SEE,	    L"Hard to see");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_BLACK_AND_WHITE,	L"Black and white");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_PETYA,              L"Pietia");
	AppendMenu(MenuTheme, MF_MENUBREAK, 0, nullptr);

	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_OFF,	   L"Edges like unlit");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_ON,	   L"Edges like lit");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_LIKE_SOLVED, L"Edges like solution");
	AppendMenu(MenuTheme, MF_STRING, MENU_THEME_EDGES_DIMMED,      L"Dimmed edges");

	AppendMenu(MenuView, MF_STRING, MENU_VIEW_SQUARES,   L"Squares");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_CIRCLES,   L"Circles");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_DIAMONDS,  L"Diamonds");
	//AppendMenu(MenuView, MF_STRING, MENU_VIEW_BEAMS,     L"Beams");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_RAINDROPS, L"Raindrops");
	AppendMenu(MenuView, MF_STRING, MENU_VIEW_CHAINS,    L"Chains");
	//AppendMenu(MenuView, MF_MENUBREAK, 0, nullptr);

	//AppendMenu(MenuView, MF_STRING, MENU_VIEW_NO_EDGES, L"Disable edges (Not implemented)");

	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE,     L"Save state 1x   size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_4X,  L"Save state 4x   size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_16X, L"Save state 16x  size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_05X, L"Save state 0.5x size...");
	AppendMenu(MenuFile, MF_STRING, MENU_FILE_SAVE_STATE_01X, L"Save state 0.1x size...");

	SetMenu(mMainWnd, mMainMenu);

	return true;
}

bool LightsOutApp::InitHotkeys()
{
	bool result = true;

	std::vector<ACCEL> accels;

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_CLICKMODE_REGULAR;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'R';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_CLICKMODE_TOROID;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'T';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_CLICKMODE_CUSTOM;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'M';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_CLICKMODE_CUSTOR;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'O';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_PERIOD_COUNT;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'V';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_PERIOD_BACK_COUNT;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'Z';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_PERIO4_COUNT;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'X';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_INCREASE_DOMAIN_SIZE;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = VK_OEM_PLUS;

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_DECREASE_DOMAIN_SIZE;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = VK_OEM_MINUS;

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_ROTATE_NONZERO;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'I';

	accels.push_back(ACCEL());
	accels.back().cmd   = HOTKEY_ID_STABLE_LIT;
	accels.back().fVirt = FCONTROL | FVIRTKEY;
	accels.back().key   = 'A';

	mAcceleratorsTable = CreateAcceleratorTable(accels.data(), accels.size());
	return mAcceleratorsTable != nullptr;
}

int LightsOutApp::RunApp()
{
	MSG msg = {0};

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if(!TranslateAccelerator(mMainWnd, mAcceleratorsTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			Update();
			mRenderer->DrawBoard(mCellSize, mGame.GetSize());
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
		mRenderer->SetColorTheme(ColorTheme::RED_EXPLOSION);
		break;
	}
	case MENU_THEME_NEON_XXL:
	{
		mRenderer->SetColorTheme(ColorTheme::NEON_XXL);
		break;
	}
	case MENU_THEME_AUTUMM:
	{
		mRenderer->SetColorTheme(ColorTheme::AUTUMM);
		break;
	}
	case MENU_THEME_CREAMED_STRAWBERRY:
	{
		mRenderer->SetColorTheme(ColorTheme::CREAMED_STRAWBERRY);
		break;
	}
	case MENU_THEME_HARD_TO_SEE:
	{
		mRenderer->SetColorTheme(ColorTheme::HARD_TO_SEE);
		break;
	}
	case MENU_THEME_BLACK_AND_WHITE:
	{
		mRenderer->SetColorTheme(ColorTheme::BLACK_AND_WHITE);
		break;
	}
	case MENU_THEME_PETYA:
	{
		mRenderer->SetColorTheme(ColorTheme::PETYA);
		break;
	}
	case MENU_THEME_EDGES_LIKE_OFF:
	{
		mRenderer->EdgeColorAsUnlit();
		break;
	}
	case MENU_THEME_EDGES_LIKE_ON:
	{
		mRenderer->EdgeColorAsLit();
		break;
	}
	case MENU_THEME_EDGES_LIKE_SOLVED:
	{
		mRenderer->EdgeColorAsSolution();
		break;
	}
	case MENU_THEME_EDGES_DIMMED:
	{
		mRenderer->EdgeColorDimmed();
		break;
	}
	case MENU_VIEW_SQUARES:
	{
		mRenderer->SetDrawTypeSquares();
		break;
	}
	case MENU_VIEW_CIRCLES:
	{
		mRenderer->SetDrawTypeCircles();
		break;
	}
	case MENU_VIEW_DIAMONDS:
	{
		mRenderer->SetDrawTypeDiamonds();
		break;
	}
	case MENU_VIEW_BEAMS:
	{
		mRenderer->SetDrawTypeBeams();
		break;
	}
	case MENU_VIEW_RAINDROPS:
	{
		mRenderer->SetDrawTypeRaindrops();
		break;
	}
	case MENU_VIEW_CHAINS:
	{
		mRenderer->SetDrawTypeChains();
		break;
	}
	case MENU_VIEW_NO_EDGES:
	{
		break;
	}
	case MENU_FILE_SAVE_STATE:
	{
		SaveBoard(EXPECTED_WND_SIZE);
		break;
	}
	case MENU_FILE_SAVE_STATE_4X:
	{
		SaveBoard(EXPECTED_WND_SIZE * 2);
		break;
	}
	case MENU_FILE_SAVE_STATE_16X:
	{
		SaveBoard(EXPECTED_WND_SIZE * 4);
		break;
	}
	case MENU_FILE_SAVE_STATE_05X:
	{
		SaveBoard(EXPECTED_WND_SIZE / 2);
		break;
	}
	case MENU_FILE_SAVE_STATE_01X:
	{
		SaveBoard(EXPECTED_WND_SIZE / 10);
		break;
	}
	}
}

void LightsOutApp::Update()
{
	//If we can take turns from the turns pool, do it
	if(mTurnList.TurnsLeft())
	{
		if(mFlags & IS_RANDOM_SOLVING)
		{
			PointOnBoard turn = mTurnList.GetRandomTurn();
			mGame.Click(turn.boardX, turn.boardY);
		}
		else
		{
			PointOnBoard turn = mTurnList.GetFirstTurn();
			mGame.Click(turn.boardX, turn.boardY);
		}

		mRenderer->SetBoardToDraw(mGame.GetBoard());
	}

	if((mFlags & IS_PERIOD_COUNTING) == 0 && (mFlags & IS_PERIOD_BACK_COUNTING) == 0 && (mFlags & IS_PERIO4_COUNTING) == 0 && mPeriodCount != 0) //Special state: no period flag is set, but period isn't zero. That means we've counted the period in the previous tick and now we should clean everything
	{
		if(mFlags & DISPLAY_PERIOD_COUNT)
		{
			mCountedBoard.Reset(mGame.GetSize());
			MessageBox(nullptr, (L"Solution period is " + std::to_wstring(mPeriodCount)).c_str(), L"Soluion period", MB_OK);
		}

		mPeriodCount = 0;
	}

	//Soiution period is being counted, redraw the board with the derived board.
	//Stop if we reached the first board.
	if(mFlags & IS_PERIOD_COUNTING)
	{
		mPeriodCount++;

		LightsOutBoard solution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetClickRule());
		mGame.Reset(solution, RESET_FLAG_LEAVE_STABILITY);

		mRenderer->SetBoardToDraw(mGame.GetBoard());
		if(mFlags & SHOW_STABILITY)
		{
			mRenderer->SetStabilityToDraw(mGame.GetStability());
		}
		else if(mFlags & SHOW_LIT_STABILITY)
		{
			LightsOutBoard stability = mGame.GetStability();
			LightsOutBoard gameBoard = mGame.GetBoard();

			stability.BoardMulComponentWise(gameBoard);
			mRenderer->SetStabilityToDraw(stability);
		}

		if (mCountedBoard == solution && (mFlags & DISPLAY_PERIOD_COUNT))
		{
			DisableFlags(IS_PERIOD_COUNTING); //Next tick we'll show the messagebox
		}
	}

	//Soiution period is being counted backwards, redraw the board with the derived board.
	//Stop if we reached the first board.
	if(mFlags & IS_PERIOD_BACK_COUNTING)
	{
		mPeriodCount++;

		LightsOutBoard invsolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetClickRule());
		mGame.Reset(invsolution, RESET_FLAG_LEAVE_STABILITY);

		mRenderer->SetBoardToDraw(mGame.GetBoard());
		if(mFlags & SHOW_STABILITY)
		{
			mRenderer->SetStabilityToDraw(mGame.GetStability());
		}
		else if(mFlags & SHOW_LIT_STABILITY)
		{
			LightsOutBoard stability = mGame.GetStability();
			LightsOutBoard gameBoard = mGame.GetBoard();

			stability.BoardMulComponentWise(gameBoard);
			mRenderer->SetStabilityToDraw(stability);
		}

		if((mFlags & DISPLAY_PERIOD_COUNT) && mCountedBoard == invsolution)
		{
			DisableFlags(IS_PERIOD_BACK_COUNTING); //Next tick we'll show the messagebox
		}
	}

	if(mFlags & IS_PERIO4_COUNTING)
	{
		LightsOutBoard solution = mGame.GetBoard();
		
		mPeriodCount++;
		solution = mSolver.GetSolution(solution, mGame.GetClickRule());
		if((mFlags & DISPLAY_PERIOD_COUNT) && mCountedBoard == solution)
		{
			DisableFlags(IS_PERIO4_COUNTING); //Next tick we'll show the messagebox
        }
		else
		{
			mPeriodCount++;
			solution = mSolver.GetSolution(solution, mGame.GetClickRule());
			if((mFlags & DISPLAY_PERIOD_COUNT) && mCountedBoard == solution)
			{
				DisableFlags(IS_PERIO4_COUNTING); //Next tick we'll show the messagebox
			}
			else
			{
				mPeriodCount++;
				solution = mSolver.GetSolution(solution, mGame.GetClickRule());
				if((mFlags & DISPLAY_PERIOD_COUNT) && mCountedBoard == solution)
				{
					DisableFlags(IS_PERIO4_COUNTING); //Next tick we'll show the messagebox
				}
				else
				{
					mPeriodCount++;
					solution = mSolver.GetSolution(solution, mGame.GetClickRule());
					if((mFlags & DISPLAY_PERIOD_COUNT) && mCountedBoard == solution)
					{
						DisableFlags(IS_PERIO4_COUNTING); //Next tick we'll show the messagebox
					}
				}
			}
		}

		mGame.Reset(solution, RESET_FLAG_LEAVE_STABILITY);
		mRenderer->SetBoardToDraw(solution);
	}

	if(mFlags & IS_EIGVEC_COUNTING)
	{
		LightsOutBoard currBoard = mGame.GetBoard();
		mGame.GetClickRule()->Click(&currBoard, mEigenvecTurn.boardX, mEigenvecTurn.boardY);

		LightsOutBoard solution = mSolver.GetSolution(currBoard, mGame.GetClickRule());
		if(currBoard == solution)
		{
			mFlags &= ~IS_EIGVEC_COUNTING;
		}

		mGame.Reset(solution, RESET_FLAG_LEAVE_STABILITY);
		mRenderer->SetBoardToDraw(mGame.GetBoard());

		if(mFlags & SHOW_STABILITY)
		{
			mRenderer->SetStabilityToDraw(mGame.GetStability());
		}
	}
}

void LightsOutApp::SaveBoard(uint32_t expectedSize)
{
	uint32_t cellSize = (uint32_t)(ceilf(expectedSize / mGame.GetSize()) - 1);

	std::vector<uint32_t> boardData;
	uint32_t boardTexRowPitch;
	mRenderer->DrawBgBoardToMemory(cellSize, mGame.GetSize(), boardData, boardTexRowPitch);

	std::wstring filePath;
	if(FileDialog::GetPictureToSave(mMainWnd, filePath))
	{
		uint32_t texSize = mGame.GetSize() * cellSize + 1;
		LightsOutSaver::SaveBMP(filePath, &boardData[0], texSize, texSize, boardTexRowPitch);
	}

	mRenderer->ResetBoardSize(mGame.GetSize());
}

void LightsOutApp::SaveBoardSilent(uint32_t expectedSize, const std::wstring& filePath)
{
	uint32_t cellSize = (uint32_t)(ceilf(expectedSize / mGame.GetSize()) - 1);

	std::vector<uint32_t> boardData;
	uint32_t boardTexRowPitch;
	mRenderer->DrawBgBoardToMemory(cellSize, mGame.GetSize(), boardData, boardTexRowPitch);

	uint32_t texSize = mGame.GetSize() * cellSize + 1;
	LightsOutSaver::SaveBMP(filePath, &boardData[0], texSize, texSize, boardTexRowPitch);

	mRenderer->ResetBoardSize(mGame.GetSize());
}

void LightsOutApp::OnMouseClick(WPARAM btnState, uint32_t xPos, uint32_t yPos)
{
	uint32_t wndSize = mGame.GetSize() * mCellSize + 1;

	int stepX = (wndSize+1) / mGame.GetSize();
	int stepY = (wndSize+1) / mGame.GetSize();

	unsigned short modX = (unsigned short)(xPos / stepX);
	unsigned short modY = (unsigned short)(yPos / stepY);

	if (mWorkingMode == WorkingMode::LIT_BOARD)
	{
		if (btnState & MK_CONTROL)
		{
			mGame.ConstructClick(modX, modY);
		}
		else if (btnState & MK_SHIFT)
		{
			if (mFlags & IS_EIGVEC_COUNTING)
			{
				mFlags &= ~IS_EIGVEC_COUNTING;
			}
			else
			{
				mFlags |= IS_EIGVEC_COUNTING;
				mEigenvecTurn = PointOnBoard(modX, modY);
			}
		}
		else
		{
			mGame.Click(modX, modY);
		}
	}
	else if(mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE || mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE_TOROID)
	{
		mGame.ConstructClick(modX, modY);
	}

	mRenderer->SetBoardToDraw(mGame.GetBoard());

	if(mFlags & SHOW_SOLUTION)
	{
		mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetClickRule());
		mRenderer->SetSolutionToDraw(mSolution);
	}
	else if(mFlags & SHOW_INVERSE_SOLUTION)
	{
		mSolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetClickRule());
		mRenderer->SetSolutionToDraw(mSolution);
	}

	if(mFlags & SHOW_STABILITY)
	{
		mRenderer->SetStabilityToDraw(mGame.GetStability());
	}
	else if(mFlags & SHOW_LIT_STABILITY)
	{
		LightsOutBoard stability = mGame.GetStability();
		LightsOutBoard gameBoard = mGame.GetBoard();

		stability.BoardMulComponentWise(gameBoard);
		mRenderer->SetStabilityToDraw(stability);
	}
}

void LightsOutApp::ChangeGameSize(int32_t newSize)
{
	ShowSolution(false);
	ShowStability(false);

	ChangeCountingMode(CountingMode::COUNT_NONE, false);
	mEigenvecTurn = PointOnBoard(-1, -1);
	mTurnList.Clear();

	Clamp(newSize, MINIMUM_FIELD_SIZE, MAXIMUM_FIELD_SIZE);
	
	if(mWorkingMode == WorkingMode::LIT_BOARD)
	{
		ResetGameBoard(ResetMode::RESET_SOLVABLE_RANDOM, newSize);

		wchar_t title[50];
		swprintf_s(title, L"Lights out %dx%d DOMAIN %d", newSize, newSize, mGame.GetDomainSize());
		SetWindowText(mMainWnd, title);

		mWindowTitle = title;
	}
	else
	{
		LightsOutBoard board(newSize, mGame.GetDomainSize());
		mGame.Reset(board, 0);

		mGame.Click(newSize / 2, newSize / 2);
		mRenderer->SetBoardToDraw(mGame.GetBoard());

		wchar_t title[50];
		swprintf_s(title, L"Lights out constructing %dx%d DOMAIN %d", newSize, newSize, mGame.GetDomainSize());
		SetWindowText(mMainWnd, title);

		mWindowTitle = title;
	}

	mRenderer->ResetBoardSize(newSize);

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
	mRenderer->OnWndResize(mWndWidth, mWndHeight);

	mRenderer->SetBoardToDraw(mGame.GetBoard());
}

void LightsOutApp::ChangeDomainSize(int32_t newDomainSize)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD)
	{
		return;
	}

	ShowSolution(false);
	ShowStability(false);

	ChangeCountingMode(CountingMode::COUNT_NONE, false);
	mEigenvecTurn = PointOnBoard(-1, -1);
	mTurnList.Clear();

	Clamp(newDomainSize, 2, 255);
	
	ResetGameBoard(ResetMode::RESET_SOLVABLE_RANDOM, mGame.GetSize(), newDomainSize);
	mGame.SetClickRuleRegular();

	wchar_t title[50];
	swprintf_s(title, L"Lights out %dx%d DOMAIN %d", mGame.GetSize(), mGame.GetSize(), newDomainSize);
	SetWindowText(mMainWnd, title);

	mWindowTitle = title;
	
	if(newDomainSize == 2)
	{
		mRenderer->SetDrawTypeBinary();
	}
	else
	{
		mRenderer->SetDrawTypeDomain();
	}

	mRenderer->ResetDomainSize(newDomainSize);
	mRenderer->SetBoardToDraw(mGame.GetBoard());
}

void LightsOutApp::ChangeWorkingMode(WorkingMode newMode)
{
	ShowSolution(false);
	ShowStability(false);

	mFlags       = 0;
	mWorkingMode = newMode;

	if(mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE || mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE_TOROID)
	{
		mSavedBoard = mGame.GetBoard();

		ClickRuleType curClickRuleType = mGame.GetClickRule()->RuleType();
		switch(curClickRuleType)
		{
		case ClickRuleType::RULE_REGULAR:
		case ClickRuleType::RULE_TOROID:
		{
			ChangeGameSize(3);
			break;
		}
		case ClickRuleType::RULE_CUSTOM:
		{
			const LightsOutClickRuleCustom* customClickRule = dynamic_cast<const LightsOutClickRuleCustom*>(mGame.GetClickRule());
			ChangeGameSize(customClickRule->RuleSize());
			break;
		}
		case ClickRuleType::RULE_CUSTOR:
		{
			const LightsOutClickRuleCustor* customClickRule = dynamic_cast<const LightsOutClickRuleCustor*>(mGame.GetClickRule());
			ChangeGameSize(customClickRule->RuleSize());
			break;
		}
		default:
			break;
		}
	}
}

void LightsOutApp::ChangeCountingMode(CountingMode cntMode, bool stopWhenReturned)
{
	if (mWorkingMode != WorkingMode::LIT_BOARD)
	{
		return;
	}

	DisableFlags(IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING);

	ShowSolution(false);
	mTurnList.Clear();

	switch(cntMode)
	{
	case CountingMode::COUNT_NONE:
		if(mFlags & DISPLAY_PERIOD_COUNT && mPeriodCount != 0)
		{
			MessageBox(nullptr, (L"Solution period so far is " + std::to_wstring(mPeriodCount)).c_str(), L"Soluion period", MB_OK);
		}
		break;
	case CountingMode::COUNT_SOLUTION_PERIOD:
		SetFlags(IS_PERIOD_COUNTING);
		break;
	case CountingMode::COUNT_SOLUTION_PERIOD_4X:
		SetFlags(IS_PERIO4_COUNTING);
		break;
	case CountingMode::COUNT_INVERSE_SOLUTION_PERIOD:
		SetFlags(IS_PERIOD_BACK_COUNTING);
		break;
	default:
		break;
	}

	mPeriodCount = 0;
	DisableFlags(DISPLAY_PERIOD_COUNT);

	if(stopWhenReturned)
	{
		SetFlags(DISPLAY_PERIOD_COUNT);
	}
	else
	{
		DisableFlags(DISPLAY_PERIOD_COUNT);
	}

	if(cntMode == CountingMode::COUNT_NONE)
	{
		mCountedBoard.Reset(mGame.GetSize());
	}
	else
	{
		mCountedBoard = mGame.GetBoard();
	}
}

void LightsOutApp::ResetGameBoard(ResetMode resetMode, uint16_t gameSize, uint16_t domainSize)
{
	mTurnList.Clear();

	if(resetMode == ResetMode::RESET_LEFT || resetMode == ResetMode::RESET_RIGHT || resetMode == ResetMode::RESET_UP || resetMode == ResetMode::RESET_DOWN)
	{
		ShowStability(false);

		auto newBoard = mGame.GetBoard();
		switch (resetMode)
		{
		case ResetMode::RESET_LEFT:
			newBoard = mSolver.MoveLeft(newBoard);
			break;
		case ResetMode::RESET_RIGHT:
			newBoard = mSolver.MoveRight(newBoard);
			break;
		case ResetMode::RESET_UP:
			newBoard = mSolver.MoveUp(newBoard);
			break;
		case ResetMode::RESET_DOWN:
			newBoard = mSolver.MoveDown(newBoard);
			break;
		default:
			break;
		}

		mGame.Reset(newBoard, 0);

		if(mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetClickRule());
			mRenderer->SetSolutionToDraw(mSolution);
		}
		else if(mFlags & SHOW_INVERSE_SOLUTION)
		{
			mSolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetClickRule());
			mRenderer->SetSolutionToDraw(mSolution);
		}
	}
	else if(resetMode == ResetMode::RESET_SOLUTION)
	{
		if (mWorkingMode != WorkingMode::LIT_BOARD)
		{
			return;
		}

		if(mFlags & SHOW_SOLUTION || mFlags & SHOW_INVERSE_SOLUTION)
		{
			mGame.Reset(mSolution, RESET_FLAG_LEAVE_STABILITY);
			if(mFlags & SHOW_LIT_STABILITY)
			{
				LightsOutBoard stability = mGame.GetStability();
				LightsOutBoard gameBoard = mGame.GetBoard();
				stability.BoardMulComponentWise(gameBoard);

				mRenderer->SetStabilityToDraw(stability);
			}
			else
			{
				mRenderer->SetStabilityToDraw(mGame.GetStability());
			}
			ShowSolution(false);
		}
		else if(mFlags & SHOW_STABILITY)
		{
			mGame.Reset(mGame.GetStability(), 0);
			mRenderer->SetStabilityToDraw(mGame.GetStability());
			ShowStability(false);
		}
		else if(mFlags & SHOW_LIT_STABILITY)
		{
			LightsOutBoard stability = mGame.GetStability();
			LightsOutBoard gameBoard = mGame.GetBoard();
			stability.BoardMulComponentWise(gameBoard);

			mGame.Reset(stability, 0);
			mRenderer->SetStabilityToDraw(mGame.GetStability());
			ShowStability(false);
		}
	}
	else if(resetMode == ResetMode::RESET_INVERTO || resetMode == ResetMode::RESET_DOMAIN_ROTATE_NONZERO)
	{
		ShowStability(false);

		LightsOutBoard inverseBoard = mGame.GetBoard();

		switch (resetMode)
		{
		case ResetMode::RESET_INVERTO:
			inverseBoard.DomainRotate();
			break;
		case ResetMode::RESET_DOMAIN_ROTATE_NONZERO:
			inverseBoard.DomainRotateNonZero();
			break;
		default:
			break;
		}

		mGame.Reset(inverseBoard, 0);
		if(mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetClickRule());
			mRenderer->SetSolutionToDraw(mSolution);
		}
		else if(mFlags & SHOW_INVERSE_SOLUTION)
		{
			mSolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetClickRule());
			mRenderer->SetSolutionToDraw(mSolution);
		}
	}
	else if(resetMode == ResetMode::RESET_SOLVABLE_RANDOM)
	{
		if(mWorkingMode != WorkingMode::LIT_BOARD)
		{
			return;
		}

		ShowStability(false);
		ShowSolution(false);

		uint16_t resetSize       = (gameSize   == 0) ? mGame.GetSize()       : gameSize;
		uint16_t resetDomainSize = (domainSize == 0) ? mGame.GetDomainSize() : domainSize;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(resetSize, resetDomainSize, BoardGenMode::BOARDGEN_FULL_RANDOM);
		newBoard = mSolver.GetInverseSolution(newBoard, mGame.GetClickRule());
		mGame.Reset(newBoard, 0);
	}
	else
	{
		ShowStability(false);
		ShowSolution(false);

		BoardGenMode modeBoardGen = BoardGenMode::BOARDGEN_ONE_ELEMENT;
		switch (resetMode)
		{
		case ResetMode::RESET_ONE:
			modeBoardGen = BoardGenMode::BOARDGEN_ONE_ELEMENT;
			break;
		case ResetMode::RESET_ZERO:
			modeBoardGen = BoardGenMode::BOARDGEN_ZERO_ELEMENT;
			break;
		case ResetMode::RESET_BORDER:
			modeBoardGen = BoardGenMode::BOARDGEN_BORDER;
			break;
		case ResetMode::RESET_PETYA:
			modeBoardGen = BoardGenMode::BOARDGEN_PETYA_STYLE;
			break;
		case ResetMode::RESET_BLATNOY:
			modeBoardGen = BoardGenMode::BOARDGEN_BLATNOY;
			break;
		case ResetMode::RESET_FULL_RANDOM:
			modeBoardGen = BoardGenMode::BOARDGEN_FULL_RANDOM;
			break;
		default:
			break;
		}

		uint16_t resetSize       = (gameSize   == 0) ? mGame.GetSize()       : gameSize;
		uint16_t resetDomainSize = (domainSize == 0) ? mGame.GetDomainSize() : domainSize;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(resetSize, resetDomainSize, modeBoardGen);
		mGame.Reset(newBoard, 0);
	}

	mRenderer->SetBoardToDraw(mGame.GetBoard());
}

void LightsOutApp::ShowSolution(bool bShow)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD)
	{
		DisableFlags(SHOW_SOLUTION | SHOW_INVERSE_SOLUTION);
		return;
	}

	mTurnList.Clear();

	DisableFlags(SHOW_INVERSE_SOLUTION);
	if(bShow)
	{
		SetFlags(SHOW_SOLUTION);

		mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetClickRule());
		mRenderer->SetSolutionToDraw(mSolution);
		mRenderer->SetSolutionVisible(true);
	}
	else
	{
		DisableFlags(SHOW_SOLUTION);

		mSolution.Reset(mGame.GetSize());
		mRenderer->SetSolutionVisible(false);
	}
}

void LightsOutApp::ShowInverseSolution(bool bShow)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD)
	{
		DisableFlags(SHOW_SOLUTION | SHOW_INVERSE_SOLUTION);
		return;
	}

	mTurnList.Clear();

	DisableFlags(SHOW_SOLUTION);
	if(bShow)
	{
		SetFlags(SHOW_INVERSE_SOLUTION);

		mSolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetClickRule());
		mRenderer->SetSolutionToDraw(mSolution);
		mRenderer->SetSolutionVisible(true);
	}
	else
	{
		DisableFlags(SHOW_INVERSE_SOLUTION);

		mSolution.Reset(mGame.GetSize());
		mRenderer->SetSolutionVisible(false);
	}
}

void LightsOutApp::ShowStability(bool bShow)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD || mGame.GetDomainSize() > 2)
	{
		DisableFlags(SHOW_STABILITY | SHOW_LIT_STABILITY);
		return;
	}

	mTurnList.Clear();

	DisableFlags(SHOW_LIT_STABILITY);
	if (bShow)
	{
		SetFlags(SHOW_STABILITY);
		ShowSolution(false);

		auto stability = mGame.GetStability();
		mRenderer->SetStabilityToDraw(stability);
		mRenderer->SetStabilityVisible(true);
	}
	else
	{
		DisableFlags(SHOW_STABILITY);
		mRenderer->SetStabilityVisible(false);
	}
}

void LightsOutApp::ShowLitStability(bool bShow)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD || mGame.GetDomainSize() > 2)
	{
		DisableFlags(SHOW_STABILITY | SHOW_LIT_STABILITY);
		return;
	}

	mTurnList.Clear();

	DisableFlags(SHOW_STABILITY);
	if(bShow)
	{
		SetFlags(SHOW_LIT_STABILITY);
		ShowSolution(false);

		auto stability = mGame.GetStability();
		stability.BoardMulComponentWise(mGame.GetBoard());

		mRenderer->SetStabilityToDraw(stability);
		mRenderer->SetStabilityVisible(true);
	}
	else
	{
		DisableFlags(SHOW_LIT_STABILITY);
		mRenderer->SetStabilityVisible(false);
	}
}

void LightsOutApp::ShowQuietPatternCount()
{
	SetWindowText(mMainWnd, (mWindowTitle + L"...").c_str());
	std::wstring wndTitle = mWindowTitle + L" (QP count: " + std::to_wstring(mSolver.QuietPatternCount(mGame.GetSize(), 2, mGame.GetClickRule())) + L")";
	SetWindowText(mMainWnd, wndTitle.c_str());
}

void LightsOutApp::IncrementGameSize()
{
	if(mWorkingMode == WorkingMode::LIT_BOARD)
	{
		ChangeGameSize((int32_t)mGame.GetSize() + 1);
	}
	else
	{
		ChangeGameSize((int32_t)mGame.GetSize() + 2);
	}
}

void LightsOutApp::DecrementGameSize()
{
	if (mWorkingMode == WorkingMode::LIT_BOARD)
	{
		ChangeGameSize((int32_t)mGame.GetSize() - 1);
	}
	else
	{
		ChangeGameSize((int32_t)mGame.GetSize() - 2);
	}
}

void LightsOutApp::CancelClickRule()
{
	if(mWorkingMode == WorkingMode::LIT_BOARD)
	{
		return;
	}

	mWorkingMode = WorkingMode::LIT_BOARD;
	mGame.SetClickRuleRegular();

	ChangeGameSize(mSavedBoard.Size());
	mGame.Reset(mSavedBoard, 0);
	mRenderer->SetBoardToDraw(mGame.GetBoard());
}

void LightsOutApp::BakeClickRule()
{
	if (mWorkingMode == WorkingMode::LIT_BOARD)
	{
		return;
	}

	if(mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE)
	{
		mGame.SetClickRuleBaked();
	}
	else if (mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE_TOROID)
	{
		mGame.SetClickRuleBakedToroid();
	}

	ChangeWorkingMode(WorkingMode::LIT_BOARD);
	ChangeGameSize(mSavedBoard.Size());

	mGame.Reset(mSavedBoard, 0);
	mRenderer->SetBoardToDraw(mGame.GetBoard());
}

void LightsOutApp::SetFlags(uint32_t FlagsMask)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD)
	{
		return;
	}

	mFlags |= FlagsMask;
}

void LightsOutApp::DisableFlags(uint32_t FlagsMask)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD)
	{
		return;
	}

	mFlags &= ~FlagsMask;
}

void LightsOutApp::ChangeFlags(uint32_t FlagsMask)
{
	if (mWorkingMode != WorkingMode::LIT_BOARD)
	{
		return;
	}

	mFlags ^= FlagsMask;
}

void LightsOutApp::SolveCurrentBoard(SolveMode solveMode)
{
	if(mWorkingMode != WorkingMode::LIT_BOARD)
	{
		return;
	}

	ShowStability(false);
	ShowSolution(false);

	if(solveMode == SolveMode::SOLVE_NONE)
	{
		mTurnList.Clear();
		DisableFlags(IS_RANDOM_SOLVING);
	}
	else
	{
		if (solveMode == SolveMode::SOLVE_ORDERED)
		{
			DisableFlags(IS_RANDOM_SOLVING);
		}
		else
		{
			SetFlags(IS_RANDOM_SOLVING);
		}

		auto solution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetClickRule());
		mTurnList.Reset(solution);
	}
}

void LightsOutApp::OnKeyReleased(WPARAM key)
{
	switch(key)
	{
	case VK_OEM_PLUS:
	{
		IncrementGameSize();
		break;
	}
	case VK_OEM_MINUS:
	{
		DecrementGameSize();
		break;
	}
	case VK_LEFT:
	{
		ResetGameBoard(ResetMode::RESET_LEFT);
		break;
	}
	case VK_RIGHT:
	{
		ResetGameBoard(ResetMode::RESET_RIGHT);
		break;
	}
	case VK_UP:
	{
		ResetGameBoard(ResetMode::RESET_UP);
		break;
	}
	case VK_DOWN:
	{
		ResetGameBoard(ResetMode::RESET_DOWN);
		break;
	}
	case VK_ESCAPE:
	{
		CancelClickRule();
		break;
	}
	case VK_RETURN:
	{
		BakeClickRule();
		break;
	}
	case 'S':
	{
		SolveCurrentBoard(!mTurnList.TurnsLeft() ? SolveMode::SOLVE_RANDOM : SolveMode::SOLVE_NONE);
		break;
	}
	case 'C':
	{
		SolveCurrentBoard(!mTurnList.TurnsLeft() ? SolveMode::SOLVE_ORDERED : SolveMode::SOLVE_NONE);
		break;
	}
	case 'R':
	{
		ResetGameBoard(ResetMode::RESET_SOLVABLE_RANDOM);
		break;
	}
	case 'F':
	{
		ResetGameBoard(ResetMode::RESET_FULL_RANDOM);
		break;
	}
	case '0':
	{
		ResetGameBoard(ResetMode::RESET_ZERO);
		break;
	}
	case '1':
	{
		ResetGameBoard(ResetMode::RESET_ONE);
		break;
	}
	case 'B':
	{
		ResetGameBoard(ResetMode::RESET_BLATNOY);
		break;
	}
	case 'P':
	{
		ResetGameBoard(ResetMode::RESET_PETYA);
		break;
	}
	case 'O':
	{
		ResetGameBoard(ResetMode::RESET_BORDER);
		break;
	}
	case 'I':
	{
		ResetGameBoard(ResetMode::RESET_INVERTO);
		break;
	}
	case 'E':
	{
		ResetGameBoard(ResetMode::RESET_SOLUTION);
		break;
	}
	case 'T':
	{
		ShowSolution(!(mFlags & SHOW_SOLUTION));
		break;
	}
	case 'W':
	{
		ShowInverseSolution(!(mFlags & SHOW_INVERSE_SOLUTION));
		break;
	}
	case 'A':
	{
		ShowStability(!(mFlags & SHOW_STABILITY));
		break;
	}
	case 'Q':
	{
		ShowQuietPatternCount();
		break;
	}
	case 'V':
	{	
		const uint32_t countFlags = IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING;
		ChangeCountingMode(!(mFlags & countFlags) ? CountingMode::COUNT_SOLUTION_PERIOD : CountingMode::COUNT_NONE, false);
		break;
	}
	case 'X':
	{
		const uint32_t countFlags = IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING;
		ChangeCountingMode(!(mFlags & countFlags) ? CountingMode::COUNT_SOLUTION_PERIOD_4X : CountingMode::COUNT_NONE, false);
		break;
	}
	case 'Z':
	{
		const uint32_t countFlags = IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING;
		ChangeCountingMode(!(mFlags & countFlags) ? CountingMode::COUNT_INVERSE_SOLUTION_PERIOD : CountingMode::COUNT_NONE, false);
		break;
	}
	}
}

void LightsOutApp::OnHotkeyPresed(WPARAM hotkey)
{
	switch (hotkey)
	{
	case HOTKEY_ID_CLICKMODE_REGULAR:
	{
		if(mWorkingMode == WorkingMode::LIT_BOARD)
		{
			ShowSolution(false);
			ShowStability(false);
			mGame.SetClickRuleRegular();
		}
		break;
	}
	case HOTKEY_ID_CLICKMODE_TOROID:
	{
		if(mWorkingMode == WorkingMode::LIT_BOARD)
		{
			ShowSolution(false);
			ShowStability(false);
			mGame.SetClickRuleToroid();
		}
		break;
	}
	case HOTKEY_ID_CLICKMODE_CUSTOM:
	{
		ChangeWorkingMode(WorkingMode::CONSTRUCT_CLICKRULE);
		break;
	}
	case HOTKEY_ID_CLICKMODE_CUSTOR:
	{
		ChangeWorkingMode(WorkingMode::CONSTRUCT_CLICKRULE_TOROID);
		break;
	}
	case HOTKEY_ID_PERIOD_COUNT:
	{
		const uint32_t countFlags = IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING;
		ChangeCountingMode(!(mFlags & countFlags) ? CountingMode::COUNT_SOLUTION_PERIOD : CountingMode::COUNT_NONE, true);
		break;
	}
	case HOTKEY_ID_PERIO4_COUNT:
	{
		const uint32_t countFlags = IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING;
		ChangeCountingMode(!(mFlags & countFlags) ? CountingMode::COUNT_SOLUTION_PERIOD_4X : CountingMode::COUNT_NONE, true);
		break;
	}
	case HOTKEY_ID_PERIOD_BACK_COUNT:
	{
		const uint32_t countFlags = IS_PERIOD_COUNTING | IS_PERIO4_COUNTING | IS_PERIOD_BACK_COUNTING;
		ChangeCountingMode(!(mFlags & countFlags) ? CountingMode::COUNT_INVERSE_SOLUTION_PERIOD : CountingMode::COUNT_NONE, true);
		break;
	}
	case HOTKEY_ID_DECREASE_DOMAIN_SIZE:
	{
		ChangeDomainSize((int32_t)mGame.GetDomainSize() - 1);
		break;
	}
	case HOTKEY_ID_INCREASE_DOMAIN_SIZE:
	{
		ChangeDomainSize((int32_t)mGame.GetDomainSize() + 1);
		break;
	}
	case HOTKEY_ID_ROTATE_NONZERO:
	{
		ResetGameBoard(ResetMode::RESET_DOMAIN_ROTATE_NONZERO);
		break;
	}
	case HOTKEY_ID_STABLE_LIT:
	{
		ShowLitStability(!(mFlags & SHOW_LIT_STABILITY));
		break;
	}
	default:
		break;
	}
}

LRESULT CALLBACK LightsOutApp::AppProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	static bool MouseHolding  = false;
	static bool HotkeyHolding = false;

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
		if(HIWORD(wParam) == 0)
		{
			OnMenuItem(wParam);
		}
		else if(HIWORD(wParam) == 1)
		{
			if(!HotkeyHolding)
			{
				OnHotkeyPresed(LOWORD(wParam));
			}
			HotkeyHolding = true;
		}

		break;
	}
	case WM_KEYUP:
	{
		if(!HotkeyHolding)
		{
			OnKeyReleased(wParam);
		}

		if(wParam != VK_CONTROL)
		{
			HotkeyHolding = false;
		}
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

