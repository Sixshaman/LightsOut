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
	uint32_t FieldSize;
	uint32_t CellSize;
	uint32_t SolveVisible;
	uint32_t StabilityVisible;

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

	static void SetFieldSize(uint32_t fieldSize);
	static void SetCellSize(uint32_t cellSize);
	static void SetSolutionVisible(bool solveVisible);
	static void SetStabilityVisible(bool stabilityVisible);

	static void SetColorNone(XMVECTOR colorNone);
	static void SetColorEnabled(XMVECTOR colorEnabled);
	static void SetColorSolved(XMVECTOR colorSolved);
	static void SetColorBetween(XMVECTOR colorBetween);

	static void SetColorBetweenAsNone();
	static void SetColorBetweenAsEnabled();
	static void SetColorBetweenAsSolved();
	static void SetColorBetweenAsDimmed();

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