#ifndef LOTEXTURES_HPP
#define LOTEXTURES_HPP

#include "LightsOut.hpp"
#include <d3d11.h>

/*
* Textures and buffers of Lights Out puzzle
*
*/
class LOTextures
{
public:
	static bool InitSRVs(ID3D11Device* Device);
	static void DestroyAll();

	static void UpdateBoard(boost::dynamic_bitset<uint32_t> board, ID3D11DeviceContext* dc);
	static void UpdateSolution(boost::dynamic_bitset<uint32_t> solve, ID3D11DeviceContext* dc);
	static void UpdateStability(boost::dynamic_bitset<uint32_t> stability, ID3D11DeviceContext* dc);

	static bool ResizeBoard(uint32_t newBoardSize, uint16_t cellSize, ID3D11Device* device);

	static ID3D11Texture2D* MappedTex(ID3D11DeviceContext* dc);

	static ID3D11ShaderResourceView* BoardSRV()     { return mBoardSRV;     };
	static ID3D11ShaderResourceView* SolutionSRV()  { return mSolutionSRV;  };
	static ID3D11ShaderResourceView* StabilitySRV() { return mStabilitySRV; };

	static ID3D11ShaderResourceView*  ResultSRV() { return mResultSRV; };
	static ID3D11UnorderedAccessView* ResultUAV() { return mResultUAV; };

private:
	static ID3D11ShaderResourceView* mBoardSRV;
	static ID3D11ShaderResourceView* mSolutionSRV;
	static ID3D11ShaderResourceView* mStabilitySRV;

	static ID3D11ShaderResourceView* mResultSRV;
	static ID3D11UnorderedAccessView* mResultUAV;

	static ID3D11ShaderResourceView*  mResultToSaveSRV;
	static ID3D11UnorderedAccessView* mResultToSaveUAV;

	static ID3D11Buffer* mBoardBuf;
	static ID3D11Buffer* mSolutionBuf;
	static ID3D11Buffer* mStabilityBuf;

	static ID3D11Texture2D* mResultCopy;
};

#endif LOTEXTURES_HPP