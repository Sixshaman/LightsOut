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