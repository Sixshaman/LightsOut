#ifndef RENDERSTATES_HPP
#define RENDERSTATES_HPP

#include <d3d11.h>
#include <wrl\client.h>

/*
* The class for some render states for drawing Lights Out board on screen
*
*/
class RenderStates
{
public:
	RenderStates(ID3D11Device *device);
	~RenderStates();

	ID3D11SamplerState* TexturePointSamplerState() { return mTexturePointSamplerState.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mTexturePointSamplerState;
};

#endif RENDERSTATES_HPP