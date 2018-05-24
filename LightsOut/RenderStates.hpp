#ifndef RENDERSTATES_HPP
#define RENDERSTATES_HPP

#include <d3d11.h>

/*
* The class for some render states for drawing Lights Out field on screen
*
*/
class RenderStates
{
public:
	static bool InitAll(ID3D11Device *device);
	static void DestroyAll();

	static ID3D11SamplerState* TextureSS() { return mTextureSS; };
private:
	static ID3D11SamplerState *mTextureSS;
};

#endif RENDERSTATES_HPP