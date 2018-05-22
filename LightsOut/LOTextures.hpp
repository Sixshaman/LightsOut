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

	static ID3D11Texture1D* mFieldTex;
	static ID3D11Texture1D* mSolveTex;

	static ID3D11Texture2D *mResultCopy;
};

#endif LOTEXTURES_HPP