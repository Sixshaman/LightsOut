#ifndef LOTEXTURES_HPP
#define LOTEXTURES_HPP

#include "..\Math\LightsOutBoard.hpp"
#include <d3d11.h>
#include <wrl\client.h>

/*
* Textures and buffers of Lights Out puzzle
*
*/
class LOTextures
{
public:
	LOTextures(ID3D11Device* Device);
	~LOTextures();

	void UpdateBoard(const LightsOutBoard& board, ID3D11DeviceContext* dc);
	void UpdateSolution(const LightsOutBoard& solution, ID3D11DeviceContext* dc);
	void UpdateStability(const LightsOutBoard& stability, ID3D11DeviceContext* dc);

	bool ResizeBoard(uint32_t newBoardSize, uint16_t cellSize, ID3D11Device* device);

	ID3D11Texture2D* MappedTex(ID3D11DeviceContext* dc);

	ID3D11ShaderResourceView* BoardSRV()     { return mBoardSRV.Get();     };
	ID3D11ShaderResourceView* SolutionSRV()  { return mSolutionSRV.Get();  };
	ID3D11ShaderResourceView* StabilitySRV() { return mStabilitySRV.Get(); };

	ID3D11ShaderResourceView*  ResultSRV() { return mResultSRV.Get(); };
	ID3D11UnorderedAccessView* ResultUAV() { return mResultUAV.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mBoardSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSolutionSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mStabilitySRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  mResultSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> mResultUAV;

	Microsoft::WRL::ComPtr<ID3D11Buffer> mBoardBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mSolutionBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mStabilityBuf;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mResultCopy;
};

#endif LOTEXTURES_HPP