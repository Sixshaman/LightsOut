#include "LOTextures.hpp"
#include "Util.hpp"

ID3D11ShaderResourceView* LOTextures::mFieldSRV = nullptr; 
ID3D11ShaderResourceView* LOTextures::mSolveSRV = nullptr; 
ID3D11ShaderResourceView* LOTextures::mResultSRV = nullptr;
ID3D11UnorderedAccessView* LOTextures::mResultUAV = nullptr; 
ID3D11ShaderResourceView* LOTextures::mResultToSaveSRV = nullptr;
ID3D11UnorderedAccessView* LOTextures::mResultToSaveUAV = nullptr;
ID3D11Texture2D* LOTextures::mResultCopy = nullptr;
ID3D11Texture1D* LOTextures::mFieldTex = nullptr;
ID3D11Texture1D* LOTextures::mSolveTex = nullptr;

bool LOTextures::InitSRVs(boost::dynamic_bitset<UINT> field, boost::dynamic_bitset<UINT> solve, UINT fieldSize, ID3D11Device* device)
{
	DestroyAll();

	UINT maxFieldSize = (UINT)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f); //Even the field of maximum size can fit here

	std::vector<UINT> fieldVec(maxFieldSize); //Compressed field
	std::vector<UINT> solveVec(maxFieldSize); //Compressed solution

	boost::to_block_range<UINT>(field, fieldVec.begin());
	boost::to_block_range<UINT>(solve, solveVec.begin());

	D3D11_TEXTURE1D_DESC fieldDesc;
	fieldDesc.Usage = D3D11_USAGE_DYNAMIC;
	fieldDesc.Width = maxFieldSize;
	fieldDesc.Format = DXGI_FORMAT_R32_UINT;
	fieldDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	fieldDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
	fieldDesc.ArraySize = 1;
	fieldDesc.MipLevels = 1;
	fieldDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA fieldData;
	fieldData.pSysMem = &fieldVec[0];
	if(FAILED(device->CreateTexture1D(&fieldDesc, &fieldData, &mFieldTex)))
	{
		MessageBox(nullptr, L"Field texture creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}
	
	fieldData.pSysMem = &solveVec[0];
	if(FAILED(device->CreateTexture1D(&fieldDesc, &fieldData, &mSolveTex)))
	{
		MessageBox(nullptr, L"Solve texture creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_UINT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvDesc.Texture1D.MipLevels = 1;
	srvDesc.Texture1D.MostDetailedMip = 0;

	if(FAILED(device->CreateShaderResourceView(mFieldTex, &srvDesc, &mFieldSRV)))
	{
		MessageBox(nullptr, L"Field SRV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if(FAILED(device->CreateShaderResourceView(mSolveTex, &srvDesc, &mSolveSRV)))
	{
		MessageBox(nullptr, L"Solve SRV creation error!", L"Error", MB_ICONERROR | MB_OK);
		return false;
	}

	if(!ResizeField(fieldSize, device))
	{
		return false;
	}

	return true;
}

void LOTextures::DestroyAll()
{
	SafeRelease(mFieldSRV);
	SafeRelease(mSolveSRV);
	SafeRelease(mResultSRV);
	SafeRelease(mResultUAV);
	SafeRelease(mResultToSaveSRV);
	SafeRelease(mResultToSaveUAV);
	SafeRelease(mFieldTex);
	SafeRelease(mSolveTex);
	SafeRelease(mResultCopy);
}

bool LOTextures::ResizeField(UINT newFieldSize, ID3D11Device* device)
{
	UINT cellSize = (UINT)(ceilf(EXPECTED_WND_SIZE / newFieldSize) - 1);

	SafeRelease(mResultSRV);
	SafeRelease(mResultUAV);
	SafeRelease(mResultCopy);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.Width = newFieldSize * cellSize + 1;
	texDesc.Height = newFieldSize * cellSize + 1;
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

void LOTextures::UpdateField(boost::dynamic_bitset<UINT> field, ID3D11DeviceContext* dc)
{
	UINT maxFieldSize = (UINT)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f);

	std::vector<UINT> fieldVec(maxFieldSize);
	boost::to_block_range<UINT>(field, fieldVec.begin());

	D3D11_MAPPED_SUBRESOURCE fieldSubresource;
	dc->Map(mFieldTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &fieldSubresource);
	UINT* fieldData = reinterpret_cast<UINT*>(fieldSubresource.pData);
	memcpy(fieldData, &fieldVec[0], maxFieldSize*sizeof(UINT));
	dc->Unmap(mFieldTex, 0);
}

void LOTextures::UpdateSolve(boost::dynamic_bitset<UINT> solve, ID3D11DeviceContext* dc)
{
	UINT maxFieldSize = (UINT)ceilf((MAXIMUM_FIELD_SIZE * MAXIMUM_FIELD_SIZE) / 32.0f);

	std::vector<UINT> solveVec(maxFieldSize);
	boost::to_block_range<UINT>(solve, solveVec.begin());

	D3D11_MAPPED_SUBRESOURCE solveSubresource;
	dc->Map(mSolveTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &solveSubresource);
	UINT* solveData = reinterpret_cast<UINT*>(solveSubresource.pData);
	memcpy(solveData, &solveVec[0], maxFieldSize*sizeof(UINT));
	dc->Unmap(mSolveTex, 0);
}

ID3D11Texture2D* LOTextures::getMappedTex(ID3D11DeviceContext* dc)
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