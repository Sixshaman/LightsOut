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

#pragma region ComputeBoard

/*
* The class for shader that writing board on off-screen texture
*
*/

class ComputeBoardShaders
{
public:
	static bool InitAll(ID3D11Device *device);
	static void DestroyAll();

	static void SetComputeBoardShader(ID3D11DeviceContext *dc);
	static void SetComputeBoardCirclesShader(ID3D11DeviceContext *dc);
	static void SetComputeBoardRaindropsShader(ID3D11DeviceContext *dc);
	static void SetComputeBoardChainsShader(ID3D11DeviceContext *dc);
	static void SetComputeEveryBoardShader(ID3D11DeviceContext *dc);
	static void SetComputeEveryBoardCirclesShader(ID3D11DeviceContext *dc);
	static void SetComputeEveryBoardRaindropsShader(ID3D11DeviceContext *dc);
	static void SetComputeEveryBoardChainsShader(ID3D11DeviceContext *dc);

private:
	static ID3D11ComputeShader* mComputeBoardShader;
	static ID3D11ComputeShader* mComputeBoardCirclesShader;
	static ID3D11ComputeShader* mComputeBoardRaindopsShader;
	static ID3D11ComputeShader* mComputeBoardChainsShader;
	static ID3D11ComputeShader* mComputeEveryBoardShader;
	static ID3D11ComputeShader* mComputeEveryBoardCirclesShader;
	static ID3D11ComputeShader* mComputeEveryBoardRaindropsShader;
	static ID3D11ComputeShader* mComputeEveryBoardChainsShader;
};

#pragma endregion ComputeBoard

#pragma region DrawScreen

/*
* The class for shaders that drawing Lights Out board on screen
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