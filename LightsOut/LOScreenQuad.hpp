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