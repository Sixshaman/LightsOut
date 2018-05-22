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

#ifndef LO_SHADERS_HPP
#define LO_SHADERS_HPP

#include <d3d11.h>

typedef void(*ShaderBindFunc)(ID3D11DeviceContext*);

#pragma region Help

/*
* The helper class for unbinding shaders from pipeline
*
*/
class ShaderBinder 
{
public:
	static void UnbindAllShaders(ID3D11DeviceContext *dc);

	static void UnbindPipelineShaders(ID3D11DeviceContext *dc);
	static void UnbindComputeShader(ID3D11DeviceContext *dc);
	static void UnbindVertexPixelShaders(ID3D11DeviceContext *dc);
};

#pragma endregion Help

#pragma region ComputeField

/*
* The class for shader that writing field on off-screen texture
*
*/

class ComputeFieldShaders
{
public:
	static bool InitAll(ID3D11Device *device);
	static void DestroyAll();

	static void SetComputeFieldShader(ID3D11DeviceContext *dc);
	static void SetComputeFieldCirclesShader(ID3D11DeviceContext *dc);
	static void SetComputeFieldRaindropsShader(ID3D11DeviceContext *dc);
	static void SetComputeFieldChainsShader(ID3D11DeviceContext *dc);

private:
	static ID3D11ComputeShader *mComputeFieldShader;
	static ID3D11ComputeShader *mComputeFieldCirclesShader;
	static ID3D11ComputeShader *mComputeFieldRaindopsShader;
	static ID3D11ComputeShader *mComputeFieldChainsShader;
};

#pragma endregion ComputeField

#pragma region DrawScreen

/*
* The class for shaders that drawing Lights Out field on screen
*
*/
class DrawScreenShaders 
{
public:
	static bool InitAll(ID3D11Device *device);
	static void DestroyAll();

	static void* GetInputSignature() { return mInputSignature; };
	static int GeiIASignatureSize() { return mInputSignatureSize; };

	static void SetPipelineShaders(ID3D11DeviceContext *dc);

private:
	static ID3D11VertexShader *mVertexShader;
	static ID3D11PixelShader  *mPixelShader;

	static void* mInputSignature;
	static int mInputSignatureSize;
};

#pragma endregion DrawScreen

#endif LO_SHADERS_HPP