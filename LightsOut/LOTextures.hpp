#ifndef LOTEXTURES_HPP
#define LOTEXTURES_HPP

#include "LightsOut.hpp"
#include <d3d11.h>

/*
* The class for textures of Lights Out puzzle
*
*/
class LOTextures
{
public:
	static bool InitSRVs(boost::dynamic_bitset<UINT> field, boost::dynamic_bitset<UINT> solve, UINT fieldSize, ID3D11Device* Device);
	static void DestroyAll();

	static void UpdateField(boost::dynamic_bitset<UINT> field, ID3D11DeviceContext* dc);
	static void UpdateSolve(boost::dynamic_bitset<UINT> solve, ID3D11DeviceContext* dc);

	static bool ResizeField(UINT newFieldSize, ID3D11Device* device);

	static ID3D11Texture2D* getMappedTex(ID3D11DeviceContext* dc);

	static ID3D11ShaderResourceView* getFieldSRV() { return mFieldSRV; };
	static ID3D11ShaderResourceView* getSolveSRV() { return mSolveSRV; };

	static ID3D11ShaderResourceView*  getResultSRV() { return mResultSRV; };
	static ID3D11UnorderedAccessView* getResultUAV() { return mResultUAV; };

private:
	static ID3D11ShaderResourceView* mFieldSRV;
	static ID3D11ShaderResourceView* mSolveSRV;

	static ID3D11ShaderResourceView* mResultSRV;
	static ID3D11UnorderedAccessView* mResultUAV;

	static ID3D11ShaderResourceView*  mResultToSaveSRV;
	static ID3D11UnorderedAccessView* mResultToSaveUAV;

	static ID3D11Texture1D* mFieldTex;
	static ID3D11Texture1D* mSolveTex;

	static ID3D11Texture2D *mResultCopy;
};

#endif LOTEXTURES_HPP