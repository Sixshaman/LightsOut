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

#ifndef LOSHADERVARIABLES_HPP
#define LOSHADERVARIABLES_HPP

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

using DirectX::XMFLOAT4;
using DirectX::XMVECTOR;

#pragma region ComputeField

struct CS_CBUFFER
{
	UINT FieldSize;
	UINT CellSize;
	bool SolveVisible;
	UINT CompressedTurn; //First 16 bit is the x-coord of hint. Second 16 bit is the y-coord of hint.

	XMFLOAT4 ColorNone;
	XMFLOAT4 ColorEnabled;
	XMFLOAT4 ColorSolved;
	XMFLOAT4 ColorBetween;
};

/*
* The class for variables of shader that writing field on off-screen texture
*
*/
class ComputeFieldVariables
{
public:
	static bool InitAll(ID3D11Device *device);
	static void DestroyAll();

	static void SetCSVariables(ID3D11DeviceContext *dc);
	static void DisableVariables(ID3D11DeviceContext* dc);

	static void SetFieldSize(UINT fieldSize);
	static void SetCellSize(UINT cellSize);
	static void SetSolveVisible(bool solveVisible);
	static void SetHintTurn(USHORT strokeX, USHORT strokeY);

	static void SetColorNone(XMVECTOR colorNone);
	static void SetColorEnabled(XMVECTOR colorEnabled);
	static void SetColorSolved(XMVECTOR colorSolved);
	static void SetColorBetween(XMVECTOR colorBetween);

	static void SetColorBetweenAsNone();
	static void SetColorBetweenAsEnabled();
	static void SetColorBetweenAsSolved();

	static void UpdateCSCBuffer(ID3D11DeviceContext *dc);

private:
	static ID3D11Buffer *mCSCbuffer;

	static CS_CBUFFER mCSCBufferCopy;
};

#pragma endregion ComputeField

#pragma region DrawScreen

/*
* The class for variables of shaders that drawing Lights Out field on screen
*
*/
class DrawScreenVariables
{
public:
	static bool InitAll(ID3D11Device *device);
	static void DestroyAll();

	static void SetAllVariables(ID3D11DeviceContext *dc);
	static void DisableVariables(ID3D11DeviceContext* dc);
};

#pragma endregion DrawScreen

#endif LOSHADERVARIABLES_HPP