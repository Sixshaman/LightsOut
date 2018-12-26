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
	SOLVE_RANDOM,
	SOLVE_ORDERED
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
	void ResetBoard(WPARAM key);
	void ChangeClickMode(WPARAM hotkey);

	void ChangeGameSize(unsigned short newSize);
	void ChangeWorkingMode(WorkingMode newMode);

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

	boost::dynamic_bitset<uint32_t> mSolution; //Solution cells

	uint32_t                        mPeriodCount;
	boost::dynamic_bitset<uint32_t> mCountedBoard;

	PointOnBoard      mEigenvecTurn;
	LightsOutTurnList mTurnList;

	int mWndWidth;
	int mWndHeight;

	uint32_t mFlags; //Some flags for showing the board
};

#endif LIGHTS_OUT_HPP