#ifndef LOSAVER_HPP
#define LOSAVER_HPP

#include <d3d11.h>
#include "LightsOutSolver.hpp"

/*
* The saver class for Lights Out game fields.
*
*/

class LightsOutSaver
{
public:
	static void SaveState(ID3D11DeviceContext* dc, HWND wndForDlg);

	//Hard to explain purpose of this method. Maybe once i will write it
	//static void SaveGameMatrix();

private:
	static void SaveBMP(std::wstring& filename, UINT* data, UINT width, UINT height, UINT rowPitch);
};

#endif LOSAVER_HPP