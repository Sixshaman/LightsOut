#ifndef LOVERTEX_HPP
#define LOVERTEX_HPP

#include <d3d11.h>
#include <DirectXMath.h>
#include "LOShaders.hpp"

struct LOVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};

/*
 * The fullscreen quad class for drawing Lights Out field on screen
 *
 */
class LOScreenQuad
{
public:
	LOScreenQuad();
	~LOScreenQuad();

	void Draw(ID3D11DeviceContext* dc);
	bool InitAll(ID3D11Device* device);

private:
	bool InitBuffers(ID3D11Device* device);
	bool InitInputLayout(ID3D11Device* device);

	LOScreenQuad(const LOScreenQuad &rhs);
	LOScreenQuad operator=(const LOScreenQuad &rhs);

private:
	ID3D11InputLayout* mInputLayout;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;
};

#endif LOVERTEX_HPP