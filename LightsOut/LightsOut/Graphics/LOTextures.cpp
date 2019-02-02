#include "LOTextures.hpp"
#include "..\Util.hpp"
#include <vector>

LOTextures::LOTextures(ID3D11Device* device)
{
	uint32_t maxBoardSize = MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE; //Even the board of maximum size can fit here

	std::vector<uint32_t> boardVec(maxBoardSize, 0); //Compressed or uncompressed board

	D3D11_BUFFER_DESC boardDesc;
	boardDesc.Usage               = D3D11_USAGE_DYNAMIC;
	boardDesc.ByteWidth           = maxBoardSize * sizeof(uint32_t);
	boardDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	boardDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
	boardDesc.MiscFlags           = 0;
	boardDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA boardData;
	boardData.pSysMem          = &boardVec[0];
	boardData.SysMemPitch      = 0;
	boardData.SysMemSlicePitch = 0;

	ThrowIfFailed(device->CreateBuffer(&boardDesc, &boardData, mBoardBuf.GetAddressOf()));
	ThrowIfFailed(device->CreateBuffer(&boardDesc, &boardData, mSolutionBuf.GetAddressOf()));
	ThrowIfFailed(device->CreateBuffer(&boardDesc, &boardData, mStabilityBuf.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format               = DXGI_FORMAT_R32_UINT;
	srvDesc.ViewDimension        = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementWidth  = sizeof(uint32_t);
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.FirstElement  = 0;
	srvDesc.Buffer.NumElements   = boardVec.size();

	ThrowIfFailed(device->CreateShaderResourceView(mBoardBuf.Get(),     &srvDesc, mBoardSRV.GetAddressOf()));
	ThrowIfFailed(device->CreateShaderResourceView(mSolutionBuf.Get(),  &srvDesc, mSolutionSRV.GetAddressOf()));
	ThrowIfFailed(device->CreateShaderResourceView(mStabilityBuf.Get(), &srvDesc, mStabilitySRV.GetAddressOf()));
}

LOTextures::~LOTextures()
{
}

void LOTextures::UpdateBoard(const LightsOutBoard& board, ID3D11DeviceContext* dc)
{
	uint32_t maxBoardSize = MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE;

	D3D11_MAPPED_SUBRESOURCE boardSubresource;
	dc->Map(mBoardBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &boardSubresource);
	uint32_t* boardData = reinterpret_cast<uint32_t*>(boardSubresource.pData);
	board.CopyMemoryData(boardData, maxBoardSize);
	dc->Unmap(mBoardBuf.Get(), 0);
}

void LOTextures::UpdateSolution(const LightsOutBoard& solution, ID3D11DeviceContext* dc)
{
	uint32_t maxBoardSize = MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE;

	D3D11_MAPPED_SUBRESOURCE solveSubresource;
	dc->Map(mSolutionBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &solveSubresource);
	uint32_t* solveData = reinterpret_cast<uint32_t*>(solveSubresource.pData);
	solution.CopyMemoryData(solveData, maxBoardSize);
	dc->Unmap(mSolutionBuf.Get(), 0);
}

void LOTextures::UpdateStability(const LightsOutBoard& stability, ID3D11DeviceContext* dc)
{
	uint32_t maxBoardSize = MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE;

	D3D11_MAPPED_SUBRESOURCE stabSubresource;
	dc->Map(mStabilityBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &stabSubresource);
	uint32_t* stabData = reinterpret_cast<uint32_t*>(stabSubresource.pData);
	stability.CopyMemoryData(stabData, maxBoardSize);
	dc->Unmap(mStabilityBuf.Get(), 0);
}

bool LOTextures::ResizeBoard(uint32_t newBoardSize, uint16_t cellSize, ID3D11Device* device)
{
	mResultSRV.Reset();
	mResultUAV.Reset();
	mResultCopy.Reset();

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Usage              = D3D11_USAGE_DEFAULT;
	texDesc.Width              = newBoardSize * cellSize + 1;
	texDesc.Height             = newBoardSize * cellSize + 1;
	texDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.SampleDesc.Count   = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.CPUAccessFlags     = 0;
	texDesc.ArraySize          = 1;
	texDesc.MipLevels          = 1;
	texDesc.MiscFlags          = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> result;
	ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, result.GetAddressOf()));

	texDesc.BindFlags      = 0;
	texDesc.Usage          = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, mResultCopy.GetAddressOf()));

	ThrowIfFailed(device->CreateShaderResourceView(result.Get(),  nullptr, mResultSRV.GetAddressOf()));
	ThrowIfFailed(device->CreateUnorderedAccessView(result.Get(), nullptr, mResultUAV.GetAddressOf()));
}

ID3D11Texture2D* LOTextures::MappedTex(ID3D11DeviceContext* dc)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> result;
	mResultSRV->GetResource(result.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> resultTex;
	result.As(&resultTex);

	dc->CopyResource(mResultCopy.Get(), resultTex.Get());

	return mResultCopy.Get();
}