#include "LightsOut.hpp"
#include "Util.hpp"
#include <WindowsX.h>
#include "LOSaver.hpp"
#include "FileDialog.hpp"
#include "LightsOutBoardGen.hpp"

#define IS_RANDOM_SOLVING       0x01 //Call LightsOutSolver::GetRandomTurn() instead of LightsOutSolver::GetFirstTurn()
#define SHOW_SOLUTION           0x02 //Show the whole solution with special color
#define SHOW_STABILITY          0x04 //Show the cell stability data
#define IS_PERIOD_COUNTING      0x08 //Replace the board with the solution each tick
#define IS_EIGVEC_COUNTING      0x10 //Replace the board with the special buffer each tick. After enough ticks you get the eigenvector of the board
#define IS_PERIO4_COUNTING      0x20 //Replace the board with the solution of the solution of the solution of the solution each tick
#define IS_PERIOD_BACK_COUNTING 0x40 //Replace the board with the anti-solution each tick

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

#define HOTKEY_ID_CLICKMODE_REGULAR 1001
#define HOTKEY_ID_CLICKMODE_TOROID  1002
#define HOTKEY_ID_CLICKMODE_CUSTOM  1003

namespace
{
	LightsOutApp *gApp = nullptr;
};

LightsOutApp::LightsOutApp(HINSTANCE hInstance): mAppInst(hInstance), mMainWnd(nullptr), mFlags(0), mWndWidth(0), 
												 mWndHeight(0), mSolver(), mPeriodCount(0), mEigenvecTurn(-1, -1)
{
	mCellSize = (uint32_t)(ceilf(EXPECTED_WND_SIZE / mGame.GetSize()) - 1);

	mWorkingMode = WorkingMode::LIT_BOARD;

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
	uint32_t wndSize = mGame.GetSize() * mCellSize + 1;

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

	HMENU MenuTheme     = CreatePopupMenu();
	HMENU MenuView      = CreatePopupMenu();
	HMENU MenuClickRule = CreatePopupMenu();
	HMENU MenuFile      = CreatePopupMenu();

	if(!mMainMenu || !MenuTheme || !MenuView || !MenuClickRule || !MenuFile)
	{
		return false;
	}

	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuTheme,     L"&Theme");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuView,      L"&View");
	AppendMenu(mMainMenu, MF_POPUP | MF_STRING, (uint32_t)MenuFile,      L"&File");

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

bool LightsOutApp::InitHotkeys()
{
	bool result = true;

	result = result && RegisterHotKey(mMainWnd, HOTKEY_ID_CLICKMODE_REGULAR, MOD_CONTROL, 'R');
	result = result && RegisterHotKey(mMainWnd, HOTKEY_ID_CLICKMODE_TOROID,  MOD_CONTROL, 'T');
	result = result && RegisterHotKey(mMainWnd, HOTKEY_ID_CLICKMODE_CUSTOM,  MOD_CONTROL, 'M');

	return result;
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
			mRenderer.DrawBoard(mCellSize, mGame.GetSize());
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

		mRenderer.SetBoardBufferData(mGame.GetBoard());
	}

	if((mFlags & IS_PERIOD_COUNTING) == 0 && (mFlags & IS_PERIOD_BACK_COUNTING) == 0 && mPeriodCount != 0) //Special state: no period flag is set, but period isn't zero. That means we've counted the period in the previous tick and now we should clean everything
	{
		mCountedBoard.clear();
		MessageBox(nullptr, (L"Solution period is " + std::to_wstring(mPeriodCount)).c_str(), L"Soluion period", MB_OK);

		mPeriodCount = 0;
	}

	//Soiution period is being counted, redraw the board with the derived board.
	//Stop if we reached the first board.
	if(mFlags & IS_PERIOD_COUNTING)
	{
		mPeriodCount++;

		boost::dynamic_bitset<uint32_t> solution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
		mGame.Reset(mGame.GetSize(), solution, RESET_FLAG_LEAVE_STABILITY);

		mRenderer.SetBoardBufferData(mGame.GetBoard());
		if (mFlags & SHOW_STABILITY)
		{
			mRenderer.SetStabilityBufferData(mGame.GetStability());
		}

		if (mCountedBoard == solution)
		{
			mFlags &= ~IS_PERIOD_COUNTING; //Next tick we'll show the messagebox
		}
	}

	//Soiution period is being counted backwards, redraw the board with the derived board.
	//Stop if we reached the first board.
	if(mFlags & IS_PERIOD_BACK_COUNTING)
	{
		mPeriodCount++;

		boost::dynamic_bitset<uint32_t> invsolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
		mGame.Reset(mGame.GetSize(), invsolution, RESET_FLAG_LEAVE_STABILITY);

		mRenderer.SetBoardBufferData(mGame.GetBoard());
		if (mFlags & SHOW_STABILITY)
		{
			mRenderer.SetStabilityBufferData(mGame.GetStability());
		}

		if (mCountedBoard == invsolution)
		{
			mFlags &= ~IS_PERIOD_BACK_COUNTING; //Next tick we'll show the messagebox
		}
	}

	if (mFlags & IS_PERIO4_COUNTING)
	{
		boost::dynamic_bitset<uint32_t> solution = mGame.GetBoard();
		
		solution = mSolver.GetSolution(solution, mGame.GetSize(), mGame.GetClickRule());
		solution = mSolver.GetSolution(solution, mGame.GetSize(), mGame.GetClickRule());
		solution = mSolver.GetSolution(solution, mGame.GetSize(), mGame.GetClickRule());
		solution = mSolver.GetSolution(solution, mGame.GetSize(), mGame.GetClickRule());

		mGame.Reset(mGame.GetSize(), solution, RESET_FLAG_LEAVE_STABILITY);
		mRenderer.SetBoardBufferData(solution);
	}

	if(mFlags & IS_EIGVEC_COUNTING)
	{
		mGame.Click(mEigenvecTurn.boardX, mEigenvecTurn.boardY);

		boost::dynamic_bitset<uint32_t> solution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
		if(mGame.GetBoard() == solution)
		{
			mFlags &= ~IS_EIGVEC_COUNTING;
		}

		mGame.Reset(mGame.GetSize(), solution, RESET_FLAG_LEAVE_STABILITY);
		mRenderer.SetBoardBufferData(mGame.GetBoard());
	}
}

void LightsOutApp::SaveBoard(uint32_t expectedSize)
{
	uint32_t cellSize = (uint32_t)(ceilf(expectedSize / mGame.GetSize()) - 1);

	std::vector<uint32_t> boardData;
	uint32_t boardTexRowPitch;
	mRenderer.DrawBgBoardToMemory(cellSize, mGame.GetSize(), boardData, boardTexRowPitch);

	std::wstring filePath;
	FileDialog::GetPictureToSave(mMainWnd, filePath);

	uint32_t texSize = mGame.GetSize() * cellSize + 1;

	LightsOutSaver::SaveBMP(filePath, &boardData[0], texSize, texSize, boardTexRowPitch);

	mRenderer.ResetBoardSize(mGame.GetSize());
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
	else if(mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE)
	{
		mGame.ConstructClick(modX, modY);
	}

	mRenderer.SetBoardBufferData(mGame.GetBoard());

	if(mFlags & SHOW_SOLUTION)
	{
		mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
		mRenderer.SetSolutionBufferData(mSolution);
	}

	if(mFlags & SHOW_STABILITY)
	{
		mRenderer.SetStabilityBufferData(mGame.GetStability());
	}
}

void LightsOutApp::ChangeGameSize(unsigned short newSize)
{
	mFlags &= ~SHOW_SOLUTION;
	mFlags &= ~IS_PERIOD_COUNTING;
	mFlags &= ~IS_PERIO4_COUNTING;
	mFlags &= ~IS_EIGVEC_COUNTING;
	mPeriodCount = 0;
	mEigenvecTurn = PointOnBoard(-1, -1);
	mCountedBoard.clear();

	Clamp(newSize, (unsigned short)MINIMUM_FIELD_SIZE, (unsigned short)MAXIMUM_FIELD_SIZE);
	
	if(mWorkingMode == WorkingMode::LIT_BOARD)
	{
		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(newSize, RESET_FULL_RANDOM);
		newBoard = mSolver.GetInverseSolution(newBoard, newSize, mGame.GetClickRule());

		mGame.Reset(newSize, newBoard, 0);

		wchar_t title[50];
		swprintf_s(title, L"Lights out %dx%d", newSize, newSize);
		SetWindowText(mMainWnd, title);
	}
	else
	{
		boost::dynamic_bitset<uint32_t> board;
		board.resize(newSize * newSize, false);
		mGame.Reset(newSize, board, 0);

		mGame.Click(newSize / 2, newSize / 2);
		mRenderer.SetBoardBufferData(mGame.GetBoard());

		wchar_t title[50];
		swprintf_s(title, L"Lights out constructing %dx%d", newSize, newSize);
		SetWindowText(mMainWnd, title);
	}

	mRenderer.ResetBoardSize(newSize);

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

	mRenderer.SetBoardBufferData(mGame.GetBoard());
}

void LightsOutApp::ChangeWorkingMode(WorkingMode newMode)
{
	mFlags = 0;
	mWorkingMode = newMode;

	if(mWorkingMode == WorkingMode::CONSTRUCT_CLICKRULE)
	{
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
		default:
			break;
		}
	}
}

void LightsOutApp::IncrementGameSize()
{
	if(mWorkingMode == WorkingMode::LIT_BOARD)
	{
		mFlags &= ~SHOW_STABILITY;
		ChangeGameSize(mGame.GetSize() + 1);
	}
	else
	{
		ChangeGameSize(mGame.GetSize() + 2);
	}
}

void LightsOutApp::DecrementGameSize()
{
	if (mWorkingMode == WorkingMode::LIT_BOARD)
	{
		mFlags &= ~SHOW_STABILITY;
		ChangeGameSize(mGame.GetSize() - 1);
	}
	else
	{
		ChangeGameSize(mGame.GetSize() - 2);
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

	ChangeGameSize(15);
}

void LightsOutApp::BakeClickRule()
{
	if (mWorkingMode == WorkingMode::LIT_BOARD)
	{
		return;
	}

	mWorkingMode = WorkingMode::LIT_BOARD;
	mGame.SetClickRuleBaked();

	ChangeGameSize(15);
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

	if(solveMode == SolveMode::SOLVE_ORDERED)
	{
		DisableFlags(IS_RANDOM_SOLVING);
		DisableFlags(SHOW_STABILITY | SHOW_SOLUTION);
	}
	else
	{
		SetFlags(IS_RANDOM_SOLVING);
		DisableFlags(SHOW_STABILITY | SHOW_SOLUTION);
	}

	auto solution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
	mTurnList.Reset(solution, mGame.GetSize());
}

void LightsOutApp::ResetBoard(WPARAM key)
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
		DisableFlags(SHOW_STABILITY);

		auto leftBoard = mGame.GetBoard();
		leftBoard = mSolver.MoveLeft(leftBoard, mGame.GetSize());
		mGame.Reset(mGame.GetSize(), leftBoard, 0);

		if(mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
			mRenderer.SetSolutionBufferData(mSolution);
		}

		break;
	}
	case VK_RIGHT:
	{
		DisableFlags(SHOW_STABILITY);

		auto rightBoard = mGame.GetBoard();
		rightBoard = mSolver.MoveRight(rightBoard, mGame.GetSize());
		mGame.Reset(mGame.GetSize(), rightBoard, 0);

		if (mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
			mRenderer.SetSolutionBufferData(mSolution);
		}

		break;
	}
	case VK_UP:
	{
		DisableFlags(SHOW_STABILITY);

		auto upBoard = mGame.GetBoard();
		upBoard = mSolver.MoveUp(upBoard, mGame.GetSize());
		mGame.Reset(mGame.GetSize(), upBoard, 0);

		if (mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
			mRenderer.SetSolutionBufferData(mSolution);
		}

		break;
	}
	case VK_DOWN:
	{
		DisableFlags(SHOW_STABILITY);

		auto downBoard = mGame.GetBoard();
		downBoard = mSolver.MoveDown(downBoard, mGame.GetSize());
		mGame.Reset(mGame.GetSize(), downBoard, 0);

		if (mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
			mRenderer.SetSolutionBufferData(mSolution);
		}

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
		SolveCurrentBoard(SolveMode::SOLVE_RANDOM);
		break;
	}
	case 'C':
	{
		SolveCurrentBoard(SolveMode::SOLVE_ORDERED);
		break;
	}
	case 'R':
	{
		DisableFlags(SHOW_STABILITY | SHOW_SOLUTION);

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_FULL_RANDOM);
		newBoard = mSolver.GetInverseSolution(newBoard, mGame.GetSize(), mGame.GetClickRule());
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case 'F':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_FULL_RANDOM);
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case '0':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_ZERO_ELEMENT);
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case '1':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_ONE_ELEMENT);
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case 'B':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_BLATNOY);
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case 'A':
	{
		DisableFlags(SHOW_SOLUTION);
		ChangeFlags(SHOW_STABILITY);

		if (mFlags & SHOW_STABILITY)
		{
			auto stability = mGame.GetStability();
			mRenderer.SetStabilityBufferData(stability);
		}
		break;
	}
	case 'P':
	{
		DisableFlags(SHOW_STABILITY | SHOW_SOLUTION);

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_PETYA_STYLE);
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case 'O':
	{
		mFlags &= ~SHOW_STABILITY;
		mFlags &= ~SHOW_SOLUTION;

		LightsOutBoardGen boardGen;
		auto newBoard = boardGen.Generate(mGame.GetSize(), RESET_BORDER);
		mGame.Reset(mGame.GetSize(), newBoard, 0);

		break;
	}
	case 'E':
	{
		if(mFlags & SHOW_SOLUTION)
		{
			mGame.Reset(mGame.GetSize(), mSolution, RESET_FLAG_LEAVE_STABILITY);
			mRenderer.SetStabilityBufferData(mGame.GetStability());
			mFlags &= ~SHOW_SOLUTION;
		}
		else if (mFlags & SHOW_STABILITY)
		{
			mGame.Reset(mGame.GetSize(), mGame.GetStability(), 0);
			mRenderer.SetStabilityBufferData(mGame.GetStability());
			mFlags &= ~SHOW_STABILITY;
		}

		break;
	}
	case 'T':
	{
		mFlags ^= SHOW_SOLUTION;
		
		if(mFlags & SHOW_SOLUTION)
		{
			mSolution = mSolver.GetSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
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
			mSolution = mSolver.GetInverseSolution(mGame.GetBoard(), mGame.GetSize(), mGame.GetClickRule());
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

		boost::dynamic_bitset<uint32_t> inverseBoard = ~(mGame.GetBoard());
		mGame.Reset(mGame.GetSize(), inverseBoard, 0);

		break;
	}
	case 'V':
	{
		mFlags &= ~IS_PERIO4_COUNTING;
		mFlags &= ~IS_PERIOD_BACK_COUNTING;

		if(!(mFlags & IS_PERIOD_COUNTING))
		{
			mFlags |= IS_PERIOD_COUNTING;
			mCountedBoard = mGame.GetBoard();
		}
		else
		{
			mFlags &= ~IS_PERIOD_COUNTING;
			mCountedBoard.clear();
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
			mCountedBoard = mGame.GetBoard();
		}
		else
		{
			mFlags &= ~IS_PERIOD_BACK_COUNTING;
			mCountedBoard.clear();
		}

		mPeriodCount = 0;

		break;
	}
	}

	mRenderer.SetSolutionVisible((mFlags & SHOW_SOLUTION) != 0);
	mRenderer.SetStabilityVisible(((mFlags & SHOW_SOLUTION) == 0) && (mFlags & SHOW_STABILITY) != 0);
	mRenderer.SetBoardBufferData(mGame.GetBoard());
}

void LightsOutApp::ChangeClickMode(WPARAM hotkey)
{
	mFlags &= ~SHOW_SOLUTION;
	mFlags &= ~SHOW_STABILITY;

	switch (hotkey)
	{
	case HOTKEY_ID_CLICKMODE_REGULAR:
		if(mWorkingMode == WorkingMode::LIT_BOARD)
		{
			mGame.SetClickRuleRegular();
		}
		break;
	case HOTKEY_ID_CLICKMODE_TOROID:
		if(mWorkingMode == WorkingMode::LIT_BOARD)
		{
			mGame.SetClickRuleToroid();
		}
		break;
	case HOTKEY_ID_CLICKMODE_CUSTOM:
		ChangeWorkingMode(WorkingMode::CONSTRUCT_CLICKRULE);
		break;
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
		OnMenuItem(wParam);
		break;
	}
	case WM_KEYUP:
	{
		if(!HotkeyHolding)
		{
			ResetBoard(wParam);
		}

		if(wParam != VK_CONTROL)
		{
			HotkeyHolding = false;
		}
		break;
	}
	case WM_HOTKEY:
	{
		if (!HotkeyHolding)
		{
			ChangeClickMode(wParam);
		}
		HotkeyHolding = true;
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

