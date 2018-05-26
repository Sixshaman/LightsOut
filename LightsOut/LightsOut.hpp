#ifndef LIGHTS_OUT_HPP
#define LIGHTS_OUT_HPP

#include <d3d11.h>
#include <Windows.h>
#include "LightsOutGame.hpp"
#include "LightsOutSolver.hpp"
#include "LORenderer.h"

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

	int RunApp();

	LRESULT CALLBACK AppProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

private:
	void OnMouseClick(WPARAM btnState, uint32_t xPos, uint32_t yPos);

	bool InitWnd();
	bool InitMenu();

	void Update();

	void OnMenuItem(WPARAM State);
	void ResetField(WPARAM key);

	void ChangeGameSize(unsigned short newSize);

private:
	HINSTANCE mAppInst;
	HWND	  mMainWnd;
	HMENU     mMainMenu;

	LightsOutRenderer mRenderer;

	LightsOutGame mGame;
	Solver mSolver;

	uint32_t mCellSize;	//Calculated from game size

	boost::dynamic_bitset<uint32_t> mSolution; //Solution cells

	uint32_t mPeriodCount;
	boost::dynamic_bitset<uint32_t> mCountedField;

	PointOnField mCurrentTurn;

	int mWndWidth;
	int mWndHeight;

	uint32_t mFlags; //Some flags for showing the field
};

#endif LIGHTS_OUT_HPP