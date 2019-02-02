#include "LOShaderVariables.hpp"
#include "..\Util.hpp"
#include "LOTextures.hpp"
#include "RenderStates.hpp"

#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02

template<typename CBufType>
inline void UpdateBuffer(ID3D11Buffer *destBuf, CBufType &srcBuf, ID3D11DeviceContext *dc)
{
	D3D11_MAPPED_SUBRESOURCE mappedbuffer;
	dc->Map(destBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedbuffer);

	CBufType *data = reinterpret_cast<CBufType*>(mappedbuffer.pData);

	memcpy(data, &srcBuf, sizeof(CBufType));

	dc->Unmap(destBuf, 0);
}

#pragma region ComputeBoard

void ComputeBoardVariables::UpdateCSCBuffer(ID3D11DeviceContext *dc)
{
	UpdateBuffer(mCSCbuffer.Get(), mCSCBufferCopy, dc);
}

ComputeBoardVariables::ComputeBoardVariables(ID3D11Device *device): mBoardSrv(nullptr), mSolutionSrv(nullptr), mStabilitySrv(nullptr), mResultUav(nullptr)
{
	ZeroMemory(&mCSCBufferCopy, sizeof(CS_CBUFFER));

	D3D11_BUFFER_DESC cBuffer;
	cBuffer.ByteWidth           = sizeof(CS_CBUFFER);
	cBuffer.Usage               = D3D11_USAGE_DYNAMIC;
	cBuffer.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	cBuffer.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cBuffer.MiscFlags           = 0;
	cBuffer.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cbInitData;
	cbInitData.pSysMem          = &mCSCBufferCopy;
	cbInitData.SysMemPitch      = 0;
	cbInitData.SysMemSlicePitch = 0;

	ThrowIfFailed(device->CreateBuffer(&cBuffer, &cbInitData, mCSCbuffer.GetAddressOf()));
}

ComputeBoardVariables::~ComputeBoardVariables()
{
}

void ComputeBoardVariables::SetCSVariables(ID3D11DeviceContext *dc)
{
	UpdateCSCBuffer(dc);

	ID3D11Buffer* cbuffers[] = {mCSCbuffer.Get()};
	dc->CSSetConstantBuffers(0, 1, cbuffers);

	ID3D11ShaderResourceView *SRVs[3] = {mBoardSrv, mSolutionSrv, mStabilitySrv};
	dc->CSSetShaderResources(0, 3, SRVs);

	ID3D11UnorderedAccessView *UAVs[1] = {mResultUav};
	dc->CSSetUnorderedAccessViews(0, 1, UAVs, nullptr);
}

void ComputeBoardVariables::DisableVariables(ID3D11DeviceContext *dc)
{
	ID3D11Buffer* nullcbuffer[] = {nullptr};
	dc->CSSetConstantBuffers(0, 1, nullcbuffer);

	ID3D11ShaderResourceView *nullsrv[3] = {nullptr, nullptr, nullptr};
	dc->CSSetShaderResources(0, 3, nullsrv);

	ID3D11UnorderedAccessView *nulluav[1] = {nullptr};
	dc->CSSetUnorderedAccessViews(0, 1, nulluav, nullptr);
}

void ComputeBoardVariables::SetBoardSrv(ID3D11ShaderResourceView* srv)
{
	mBoardSrv = srv;
}

void ComputeBoardVariables::SetSolutionSrv(ID3D11ShaderResourceView* srv)
{
	mSolutionSrv = srv;
}

void ComputeBoardVariables::SetStabilitySrv(ID3D11ShaderResourceView* srv)
{
	mStabilitySrv = srv;
}

void ComputeBoardVariables::SetResultUav(ID3D11UnorderedAccessView* uav)
{
	mResultUav = uav;
}

void ComputeBoardVariables::SetBoardSize(uint32_t boardSize)
{
	mCSCBufferCopy.BoardSize = boardSize;
}

void ComputeBoardVariables::SetCellSize(uint32_t cellSize)
{
	mCSCBufferCopy.CellSize = cellSize;
}

void ComputeBoardVariables::SetDomainSize(uint32_t domainSize)
{
	mCSCBufferCopy.DomainSize = domainSize;
}

void ComputeBoardVariables::SetSolutionVisible(bool solveVisible)
{
	mCSCBufferCopy.Flags = (int)solveVisible * (mCSCBufferCopy.Flags | FLAG_SHOW_SOLUTION) + (int)(!solveVisible) * (mCSCBufferCopy.Flags & ~FLAG_SHOW_SOLUTION);
}

void ComputeBoardVariables::SetStabilityVisible(bool stabilityVisible)
{
	mCSCBufferCopy.Flags = (int)stabilityVisible * (mCSCBufferCopy.Flags | FLAG_SHOW_STABILITY) + (int)(!stabilityVisible) * (mCSCBufferCopy.Flags & ~FLAG_SHOW_STABILITY);
}

void ComputeBoardVariables::SetColorNone(DirectX::XMVECTOR colorNone)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorNone, colorNone);
}

void ComputeBoardVariables::SetColorEnabled(DirectX::XMVECTOR colorEnabled)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorEnabled,colorEnabled);
}

void ComputeBoardVariables::SetColorSolved(DirectX::XMVECTOR colorSolved)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorSolved, colorSolved);
}

void ComputeBoardVariables::SetColorBetween(DirectX::XMVECTOR colorBetween)
{
	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorBetween, colorBetween);
}

void ComputeBoardVariables::SetColorBetweenAsNone()
{
	mCSCBufferCopy.ColorBetween = mCSCBufferCopy.ColorNone;
}

void ComputeBoardVariables::SetColorBetweenAsEnabled()
{
	mCSCBufferCopy.ColorBetween = mCSCBufferCopy.ColorEnabled;
}

void ComputeBoardVariables::SetColorBetweenAsSolved()
{
	mCSCBufferCopy.ColorBetween = mCSCBufferCopy.ColorSolved;
}

void ComputeBoardVariables::SetColorBetweenAsDimmed()
{
	DirectX::XMVECTOR unlitColor    = DirectX::XMLoadFloat4(&mCSCBufferCopy.ColorNone);
	DirectX::XMVECTOR invUnlitColor = DirectX::XMVectorSubtract(DirectX::XMVectorSplatOne(), unlitColor);

	DirectX::XMVECTOR halfLit = DirectX::XMVectorScale(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(invUnlitColor, unlitColor)), 0.5f);

	DirectX::XMStoreFloat4(&mCSCBufferCopy.ColorBetween, halfLit);
}

#pragma endregion ComputeBoard

#pragma region DrawScreen

DrawScreenVariables::DrawScreenVariables(ID3D11Device *device): mTextureSamplerState(nullptr), mBoardTextureSrv(nullptr)
{
}

DrawScreenVariables::~DrawScreenVariables()
{
}

void DrawScreenVariables::SetAllVariables(ID3D11DeviceContext *dc)
{
	ID3D11ShaderResourceView* targetTex[] = {mBoardTextureSrv};
	dc->PSSetShaderResources(0, 1, targetTex);

	ID3D11SamplerState* TextureSS[] = {mTextureSamplerState};
	dc->PSSetSamplers(0, 1, TextureSS);
}

void DrawScreenVariables::DisableVariables(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView *nullsrv[1] = {nullptr};
	dc->PSSetShaderResources(0, 1, nullsrv);

	ID3D11SamplerState *nullsampler[1] = {nullptr};
	dc->PSSetSamplers(0, 1, nullsampler);
}

void DrawScreenVariables::SetBoardTexture(ID3D11ShaderResourceView* boardTexture)
{
	mBoardTextureSrv = boardTexture;
}

void DrawScreenVariables::SetSamplerState(ID3D11SamplerState* samplerState)
{
	mTextureSamplerState = samplerState;
}

#pragma endregion DrawScreen