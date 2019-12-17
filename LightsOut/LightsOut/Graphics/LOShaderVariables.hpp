#ifndef LOSHADERVARIABLES_HPP
#define LOSHADERVARIABLES_HPP

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl\client.h>

#pragma region ComputeBoard

/*
* The class for shader variables to write the board on off-screen texture
*
*/
class ComputeBoardVariables
{
public:
	ComputeBoardVariables(ID3D11Device *device);
	~ComputeBoardVariables();

	void SetCSVariables(ID3D11DeviceContext *dc);
	void DisableVariables(ID3D11DeviceContext* dc);

	void SetBoardSrv(ID3D11ShaderResourceView* srv);
	void SetSolutionSrv(ID3D11ShaderResourceView* srv);
	void SetStabilitySrv(ID3D11ShaderResourceView* srv);

	void SetResultUav(ID3D11UnorderedAccessView* uav);

	void SetBoardSize(uint32_t boardSize);
	void SetCellSize(uint32_t cellSize);
	void SetDomainSize(uint32_t domainSize);
	void SetSolutionVisible(bool solveVisible);
	void SetStabilityVisible(bool stabilityVisible);
	void SetToroidRender(bool toroidRender);

	void SetColorNone(DirectX::XMVECTOR colorNone);
	void SetColorEnabled(DirectX::XMVECTOR colorEnabled);
	void SetColorSolved(DirectX::XMVECTOR colorSolved);
	void SetColorBetween(DirectX::XMVECTOR colorBetween);

	void SetColorBetweenAsNone();
	void SetColorBetweenAsEnabled();
	void SetColorBetweenAsSolved();
	void SetColorBetweenAsDimmed();

	void UpdateCSCBuffer(ID3D11DeviceContext *dc);

private:
	struct CS_CBUFFER
	{
		uint32_t BoardSize;
		uint32_t CellSize;
		uint32_t DomainSize;
		uint32_t Flags;

		DirectX::XMFLOAT4 ColorNone;
		DirectX::XMFLOAT4 ColorEnabled;
		DirectX::XMFLOAT4 ColorSolved;
		DirectX::XMFLOAT4 ColorBetween;
	};

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> mCSCbuffer;

	CS_CBUFFER mCSCBufferCopy;

	ID3D11ShaderResourceView* mBoardSrv;
	ID3D11ShaderResourceView* mSolutionSrv;
	ID3D11ShaderResourceView* mStabilitySrv;

	ID3D11UnorderedAccessView* mResultUav;
};

#pragma endregion ComputeBoard

#pragma region DrawScreen

/*
* The class for shader variables to draw Lights Out board texture on screen
*
*/
class DrawScreenVariables
{
public:
	DrawScreenVariables(ID3D11Device *device);
	~DrawScreenVariables();

	void SetAllVariables(ID3D11DeviceContext *dc);
	void DisableVariables(ID3D11DeviceContext* dc);

	void SetBoardTexture(ID3D11ShaderResourceView* boardTexture);

	void SetSamplerState(ID3D11SamplerState* samplerState);

private:
	ID3D11SamplerState* mTextureSamplerState;

	ID3D11ShaderResourceView* mBoardTextureSrv;
};

#pragma endregion DrawScreen

#endif LOSHADERVARIABLES_HPP