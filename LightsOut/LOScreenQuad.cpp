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

#include "LOScreenQuad.hpp"
#include "LOShaders.hpp"
#include "LOShaderVariables.hpp"
#include "Util.hpp"

LOScreenQuad::LOScreenQuad(): mScreenQuadVB(nullptr), mScreenQuadIB(nullptr), mInputLayout(nullptr)
{

}

LOScreenQuad::~LOScreenQuad()
{
	SafeRelease(mScreenQuadVB);
	SafeRelease(mScreenQuadIB);

	SafeRelease(mInputLayout);
}

bool LOScreenQuad::InitAll(ID3D11Device* device)
{
	if(!InitBuffers(device))
	{
		return false;
	}

	if(!InitInputLayout(device))
	{
		return false;
	}

	return true;
}

bool LOScreenQuad::InitBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = 4*sizeof(LOVertex);
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	LOVertex vertices[4] = 
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}
	};

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;

	if(FAILED(device->CreateBuffer(&vbDesc, &vinitData, &mScreenQuadVB)))
	{
		MessageBox(nullptr, L"Vertex buffer creating error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = 6*sizeof(USHORT);
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	USHORT indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	if(FAILED(device->CreateBuffer(&ibDesc, &iinitData, &mScreenQuadIB)))
	{
		MessageBox(nullptr, L"Index buffer creating error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	return true;
}

bool LOScreenQuad::InitInputLayout(ID3D11Device* device)
{
	D3D11_INPUT_ELEMENT_DESC LOVertexDesc[2] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	if(FAILED(device->CreateInputLayout(LOVertexDesc, 2, DrawScreenShaders::GetInputSignature(), 
										DrawScreenShaders::GeiIASignatureSize(), &mInputLayout)))
	{
		MessageBox(nullptr, L"Input layout creating error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	return true;
}

void LOScreenQuad::Draw(ID3D11DeviceContext* dc)
{
	UINT stride = sizeof(LOVertex);
	UINT offset = 0;

	dc->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	dc->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	dc->IASetInputLayout(mInputLayout);

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DrawScreenVariables::SetAllVariables(dc);
	DrawScreenShaders::SetPipelineShaders(dc);

	dc->DrawIndexed(6, 0, 0);

	DrawScreenVariables::DisableVariables(dc);
	ShaderBinder::UnbindPipelineShaders(dc);
}