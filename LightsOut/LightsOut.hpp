#ifndef LIGHTS_OUT_HPP
#define LIGHTS_OUT_HPP

#include <d3d11.h>
#include <Windows.h>
#include "LightsOutGame.hpp"
#include "LightsOutSolver.hpp"
#include "LightsOutTurnList.hpp"
#include "LORenderer.h"

/*
* The main class for Lights Out game.
*
*/

enum class WorkingMode
{
	LIT_BOARD,
	CONSTRUCT_CLICKRULE,
	CONSTRUCT_CLICKRULE_TOROID
};

enum class SolveMode
{
	SOLVE_NONE,
	SOLVE_RANDOM,
	SOLVE_ORDERED
};

enum class ResetMode
{
	RESET_ONE,
	RESET_ZERO,
	RESET_BORDER,
	RESET_PETYA,
	RESET_BLATNOY,
	RESET_SOLVABLE_RANDOM,
	RESET_FULL_RANDOM,
	RESET_SOLUTION,
	RESET_INVERTO,
	RESET_LEFT,
	RESET_RIGHT,
	RESET_UP,
	RESET_DOWN
};

enum class CountingMode
{
	COUNT_NONE,
	COUNT_SOLUTION_PERIOD,
	COUNT_INVERSE_SOLUTION_PERIOD,
	COUNT_SOLUTION_PERIOD_4X
};

class LightsOutApp
{
public:
	LightsOutApp(HINSTANCE hInstance);
	~LightsOutApp();

	bool InitAll();

	int RunApp();

	LRESULT CALLBACK AppProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

private:
	void OnMouseClick(WPARAM btnState, uint32_t xPos, uint32_t yPos);

	bool InitWnd();
	bool InitMenu();
	bool InitHotkeys();

	void Update();

	void SaveBoard(uint32_t expectedSize);

	void OnMenuItem(WPARAM State);
	void OnKeyReleased(WPARAM key);
	void OnHotkeyPresed(WPARAM hotkey);

	void ChangeGameSize(unsigned short newSize);
	void ChangeWorkingMode(WorkingMode newMode);
	void ChangeCountingMode(CountingMode cntMode);

	void ResetGameBoard(ResetMode resetMode, uint16_t gameSize = 0);

	void ShowSolution(bool bShow);
	void ShowInverseSolution(bool bShow);
	void ShowStability(bool bShow);

	void ShowQuietPatternCount();

	void IncrementGameSize();
	void DecrementGameSize();

	void CancelClickRule();
	void BakeClickRule();

	void SetFlags(uint32_t FlagsMask);
	void DisableFlags(uint32_t FlagsMask);
	void ChangeFlags(uint32_t FlagsMask);

	void SolveCurrentBoard(SolveMode solveMode);

private:
	HINSTANCE mAppInst;
	HWND	  mMainWnd;
	HMENU     mMainMenu;

	LightsOutRenderer mRenderer;

	LightsOutGame   mGame;
	LightsOutSolver mSolver;

	uint32_t    mCellSize;    //Calculated from game size
	WorkingMode mWorkingMode; //Current working mode

	uint32_t                        mPeriodCount;
	boost::dynamic_bitset<uint32_t> mCountedBoard;

	boost::dynamic_bitset<uint32_t> mSolution; //Solution cells

	PointOnBoard      mEigenvecTurn;
	LightsOutTurnList mTurnList;

	int          mWndWidth;
	int          mWndHeight;
	std::wstring mWindowTitle;

	uint32_t mFlags; //Some flags for showing the board
};

#endif LIGHTS_OUT_HPP