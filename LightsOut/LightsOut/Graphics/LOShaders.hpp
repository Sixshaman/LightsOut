#ifndef LO_SHADERS_HPP
#define LO_SHADERS_HPP

#include <d3d11.h>
#include <wrl/client.h>

#pragma region ComputeBoard

/*
* The class for shader that writing board on off-screen texture
*
*/

class ComputeBoardShaders
{
public:
	ComputeBoardShaders(ID3D11Device* device);
	~ComputeBoardShaders();

	void SetComputeBoardShader(ID3D11DeviceContext* dc);
	void SetComputeBoardCirclesShader(ID3D11DeviceContext* dc);
	void SetComputeBoardDiamondsShader(ID3D11DeviceContext* dc);
	void SetComputeBoardBeamsShader(ID3D11DeviceContext* dc);
	void SetComputeBoardRaindropsShader(ID3D11DeviceContext* dc);
	void SetComputeBoardChainsShader(ID3D11DeviceContext* dc);

	void SetComputeEveryBoardShader(ID3D11DeviceContext* dc);
	void SetComputeEveryBoardCirclesShader(ID3D11DeviceContext* dc);
	void SetComputeEveryBoardDiamondsShader(ID3D11DeviceContext* dc);
	void SetComputeEveryBoardBeamsShader(ID3D11DeviceContext* dc);
	void SetComputeEveryBoardRaindropsShader(ID3D11DeviceContext* dc);
	void SetComputeEveryBoardChainsShader(ID3D11DeviceContext* dc);

	void UnbindComputeShader(ID3D11DeviceContext* dc);

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeBoardShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeBoardCirclesShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeBoardDiamondsShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeBoardBeamsShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeBoardRaindopsShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeBoardChainsShader;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeEveryBoardShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeEveryBoardCirclesShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeEveryBoardDiamondsShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeEveryBoardBeamsShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeEveryBoardRaindropsShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mComputeEveryBoardChainsShader;
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
	DrawScreenShaders(ID3D11Device *device);
	~DrawScreenShaders();

	void SetPipelineShaders(ID3D11DeviceContext* dc);
	void UnbindPipelineShaders(ID3D11DeviceContext* dc);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  mPixelShader;
};

#pragma endregion DrawScreen

#endif LO_SHADERS_HPP