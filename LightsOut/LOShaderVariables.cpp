#include "LOShaderVariables.hpp"
#include "Util.hpp"
#include "LOTextures.hpp"
#include "RenderStates.hpp"

template<typename CBufType>
inline void UpdateBuffer(ID3D11Buffer *destBuf, CBufType &srcBuf, ID3D11DeviceContext *dc)
{
	D3D11_MAPPED_SUBRESOURCE mappedbuffer;
	dc->Map(destBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedbuffer);

	CBufType *data = reinterpret_cast<CBufType*>(mappedbuffer.pData);

	memcpy(data, &srcBuf, sizeof(CBufType));

	dc->Unmap(destBuf, 0);
}

#pragma region ComputeField

ID3D11Buffer* ComputeFieldVariables::mCSCbuffer = nullptr;

CS_CBUFFER ComputeFieldVariables::mCSCBufferCopy = {};

void ComputeFieldVariables::UpdateCSCBuffer(ID3D11DeviceContext *dc)
{
	UpdateBuffer(mCSCbuffer, mCSCBufferCopy, dc);
}

bool ComputeFieldVariables::InitAll(ID3D11Device *device)
{
	ZeroMemory(&mCSCBufferCopy, sizeof(CS_CBUFFER));

	D3D11_BUFFER_DESC cBuffer;
	cBuffer.ByteWidth = sizeof(CS_CBUFFER);
	cBuffer.Usage = D3D11_USAGE_DYNAMIC;
	cBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBuffer.MiscFlags = 0;
	cBuffer.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cbInitData;
	cbInitData.pSysMem = &mCSCBufferCopy;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;

	ASSERT_ERR(device->CreateBuffer(&cBuffer, &cbInitData, &mCSCbuffer));

	return true;
}

void ComputeFieldVariables::DestroyAll()
{
	SafeRelease(mCSCbuffer);
}

void ComputeFieldVariables::SetCSVariables(ID3D11DeviceContext *dc)
{
	UpdateCSCBuffer(dc);

	dc->CSSetConstantBuffers(0, 1, &mCSCbuffer);

	ID3D11ShaderResourceView *SRVs[2] = {LOTextures::getFieldSRV(), LOTextures::getSolutionSRV()};
	dc->CSSetShaderResources(0, 2, SRVs);

	ID3D11UnorderedAccessView *UAVs[1] = {LOTextures::getResultUAV()};
	dc->CSSetUnorderedAccessViews(0, 1, UAVs, nullptr);
}

void ComputeFieldVariables::DisableVariables(ID3D11DeviceContext *dc)
{
	ID3D11Buffer* nullcbuffer = {nullptr};
	dc->CSSetConstantBuffers(0, 1, &nullcbuffer);

	ID3D11ShaderResourceView *nullsrv[2] = {nullptr, nullptr};
	dc->CSSetShaderResources(0, 2, nullsrv);

	ID3D11UnorderedAccessView *nulluav[1] = {nullptr};
	dc->CSSetUnorderedAccessViews(0, 1, nulluav, nullptr);
}

void ComputeFieldVariables::SetFieldSize(uint32_t fieldSize)
{
	mCSCBufferCopy.FieldSize = fieldSize;
}

void ComputeFieldVariables::SetCellSize(uint32_t cellSize)
{
	mCSCBufferCopy.CellSize = cellSize;
}

void ComputeFieldVariables::SetSolutionVisible(bool solveVisible)
{
	mCSCBufferCopy.SolveVisible = solveVisible;
}

void ComputeFieldVariables::SetHintTurn(USHORT strokeX, USHORT strokeY)
{
	mCSCBufferCopy.CompressedTurn = ((uint32_t)strokeX << 16) | strokeY;
}

void ComputeFieldVariables::SetColorNone(XMVECTOR colorNone)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorNone, colorNone);
}

void ComputeFieldVariables::SetColorEnabled(XMVECTOR colorEnabled)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorEnabled,colorEnabled);
}

void ComputeFieldVariables::SetColorSolved(XMVECTOR colorSolved)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorSolved, colorSolved);
}

void ComputeFieldVariables::SetColorBetween(XMVECTOR colorBetween)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorBetween, colorBetween);
}

void ComputeFieldVariables::SetColorBetweenAsNone()
{
	mCSCBufferCopy.ColorBetween = mCSCBufferCopy.ColorNone;
}

void ComputeFieldVariables::SetColorBetweenAsEnabled()
{
	mCSCBufferCopy.ColorBetween = mCSCBufferCopy.ColorEnabled;
}

void ComputeFieldVariables::SetColorBetweenAsSolved()
{
	mCSCBufferCopy.ColorBetween = mCSCBufferCopy.ColorSolved;
}

void ComputeFieldVariables::SetColorBetweenAsDimmed()
{
	XMVECTOR unlitColor    = DirectX::XMLoadFloat4(&mCSCBufferCopy.ColorNone);
	XMVECTOR invUnlitColor = DirectX::XMVectorSubtract(DirectX::XMVectorSplatOne(), unlitColor);

	XMVECTOR halfLit = DirectX::XMVectorScale(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(invUnlitColor, unlitColor)), 0.5f);

	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorBetween, halfLit);
}

#pragma endregion ComputeField

#pragma region DrawScreen

bool DrawScreenVariables::InitAll(ID3D11Device *device)
{
	return true;
}

void DrawScreenVariables::DestroyAll()
{
}

void DrawScreenVariables::SetAllVariables(ID3D11DeviceContext *dc)
{
	ID3D11ShaderResourceView* targetTex = LOTextures::getResultSRV();
	dc->PSSetShaderResources(0, 1, &targetTex);

	ID3D11SamplerState* TextureSS = RenderStates::TextureSS();
	dc->PSSetSamplers(0, 1, &TextureSS);
}

void DrawScreenVariables::DisableVariables(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView *nullsrv[1] = {nullptr};
	dc->PSSetShaderResources(0, 1, nullsrv);

	ID3D11SamplerState *nullsampler[1] = {nullptr};
	dc->PSSetSamplers(0, 1, nullsampler);
}

#pragma endregion DrawScreen