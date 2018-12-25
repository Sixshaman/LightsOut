#include "LOTextures.hpp"
#include "Util.hpp"

ID3D11ShaderResourceView* LOTextures::mBoardSRV = nullptr; 
ID3D11ShaderResourceView* LOTextures::mSolutionSRV = nullptr; 
ID3D11ShaderResourceView* LOTextures::mStabilitySRV = nullptr;
ID3D11ShaderResourceView* LOTextures::mResultSRV = nullptr;
ID3D11UnorderedAccessView* LOTextures::mResultUAV = nullptr; 
ID3D11ShaderResourceView* LOTextures::mResultToSaveSRV = nullptr;
ID3D11UnorderedAccessView* LOTextures::mResultToSaveUAV = nullptr;
ID3D11Buffer* LOTextures::mBoardBuf = nullptr;
ID3D11Buffer* LOTextures::mSolutionBuf = nullptr;
ID3D11Buffer* LOTextures::mStabilityBuf = nullptr;
ID3D11Texture2D* LOTextures::mResultCopy = nullptr;

bool LOTextures::InitSRVs(ID3D11Device* device)
{
	DestroyAll();

	uint32_t maxBoardSize = (uint32_t)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f); //Even the board of maximum size can fit here

	std::vector<uint32_t> boardVec(maxBoardSize, 0); //Compressed board

	D3D11_BUFFER_DESC boardDesc;
	boardDesc.Usage               = D3D11_USAGE_DYNAMIC;
	boardDesc.ByteWidth           = maxBoardSize * sizeof(uint32_t);
	boardDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	boardDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
	boardDesc.MiscFlags           = 0;
	boardDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA boardData;
	boardData.pSysMem = &boardVec[0];
	if(FAILED(device->CreateBuffer(&boardDesc, &boardData, &mBoardBuf)))
	{
		MessageBox(nullptr, L"Board texture creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}
	
	boardData.pSysMem = &boardVec[0];
	if(FAILED(device->CreateBuffer(&boardDesc, &boardData, &mSolutionBuf)))
	{
		MessageBox(nullptr, L"Solve texture creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	boardData.pSysMem = &boardVec[0];
	if (FAILED(device->CreateBuffer(&boardDesc, &boardData, &mStabilityBuf)))
	{
		MessageBox(nullptr, L"Solve texture creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format               = DXGI_FORMAT_R32_UINT;
	srvDesc.ViewDimension        = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement  = 0;
	srvDesc.Buffer.NumElements   = boardVec.size();

	if(FAILED(device->CreateShaderResourceView(mBoardBuf, &srvDesc, &mBoardSRV)))
	{
		MessageBox(nullptr, L"Board SRV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if(FAILED(device->CreateShaderResourceView(mSolutionBuf, &srvDesc, &mSolutionSRV)))
	{
		MessageBox(nullptr, L"Solve SRV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if (FAILED(device->CreateShaderResourceView(mStabilityBuf, &srvDesc, &mStabilitySRV)))
	{
		MessageBox(nullptr, L"Stability SRV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	return true;
}

void LOTextures::DestroyAll()
{
	SafeRelease(mBoardSRV);
	SafeRelease(mSolutionSRV);
	SafeRelease(mStabilitySRV);
	SafeRelease(mResultSRV);
	SafeRelease(mResultUAV);
	SafeRelease(mResultToSaveSRV);
	SafeRelease(mResultToSaveUAV);
	SafeRelease(mBoardBuf);
	SafeRelease(mSolutionBuf);
	SafeRelease(mStabilityBuf);
	SafeRelease(mResultCopy);
}

bool LOTextures::ResizeBoard(uint32_t newBoardSize, uint16_t cellSize, ID3D11Device* device)
{
	SafeRelease(mResultSRV);
	SafeRelease(mResultUAV);
	SafeRelease(mResultCopy);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.Width = newBoardSize * cellSize + 1;
	texDesc.Height = newBoardSize * cellSize + 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.CPUAccessFlags = 0;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* result = nullptr;
	if(FAILED(device->CreateTexture2D(&texDesc, nullptr, &result)))
	{
		MessageBox(nullptr, L"Result texture creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	texDesc.BindFlags = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	if(FAILED(device->CreateTexture2D(&texDesc, nullptr, &mResultCopy)))
	{
		MessageBox(nullptr, L"Result copy creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if(FAILED(device->CreateShaderResourceView(result, nullptr, &mResultSRV)))
	{
		MessageBox(nullptr, L"Result SRV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}
	if(FAILED(device->CreateUnorderedAccessView(result, nullptr, &mResultUAV)))
	{
		MessageBox(nullptr, L"Result UAV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	SafeRelease(result);

	return true;
}

void LOTextures::UpdateBoard(boost::dynamic_bitset<uint32_t> board, ID3D11DeviceContext* dc)
{
	uint32_t maxBoardSize = (uint32_t)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f);

	std::vector<uint32_t> boardVec(maxBoardSize);
	boost::to_block_range<uint32_t>(board, boardVec.begin());

	D3D11_MAPPED_SUBRESOURCE boardSubresource;
	dc->Map(mBoardBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &boardSubresource);
	uint32_t* boardData = reinterpret_cast<uint32_t*>(boardSubresource.pData);
	memcpy(boardData, &boardVec[0], maxBoardSize*sizeof(uint32_t));
	dc->Unmap(mBoardBuf, 0);
}

void LOTextures::UpdateSolution(boost::dynamic_bitset<uint32_t> solution, ID3D11DeviceContext* dc)
{
	uint32_t maxBoardSize = (uint32_t)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f);

	std::vector<uint32_t> solveVec(maxBoardSize);
	boost::to_block_range<uint32_t>(solution, solveVec.begin());

	D3D11_MAPPED_SUBRESOURCE solveSubresource;
	dc->Map(mSolutionBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &solveSubresource);
	uint32_t* solveData = reinterpret_cast<uint32_t*>(solveSubresource.pData);
	memcpy(solveData, &solveVec[0], maxBoardSize*sizeof(uint32_t));
	dc->Unmap(mSolutionBuf, 0);
}

void LOTextures::UpdateStability(boost::dynamic_bitset<uint32_t> stability, ID3D11DeviceContext * dc)
{
	uint32_t maxBoardSize = (uint32_t)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f);

	std::vector<uint32_t> stabVec(maxBoardSize);
	boost::to_block_range<uint32_t>(stability, stabVec.begin());

	D3D11_MAPPED_SUBRESOURCE stabSubresource;
	dc->Map(mStabilityBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &stabSubresource);
	uint32_t* stabData = reinterpret_cast<uint32_t*>(stabSubresource.pData);
	memcpy(stabData, &stabVec[0], maxBoardSize * sizeof(uint32_t));
	dc->Unmap(mStabilityBuf, 0);
}

ID3D11Texture2D* LOTextures::MappedTex(ID3D11DeviceContext* dc)
{
	ID3D11Resource *result = nullptr;
	mResultSRV->GetResource(&result);

	ID3D11Texture2D *resultTex = nullptr;
	result->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&resultTex));

	if(!result || !resultTex)
	{
		MessageBox(NULL, L"Error getting resource", L"Test", MB_OK);
		return nullptr;
	}

	dc->CopyResource(mResultCopy, resultTex);

	SafeRelease(result);
	SafeRelease(resultTex);

	return mResultCopy;
}