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