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

#include "LOSaver.hpp"
#include "LOTextures.hpp"
#include "FileDialog.hpp"

void LightsOutSaver::SaveState(ID3D11DeviceContext* dc, HWND wndForDlg)
{
	std::wstring filePath;
	FileDialog::GetPictureToSave(wndForDlg, filePath);

	ID3D11Texture2D* resultTex = LOTextures::getMappedTex(dc);

	if(!resultTex)
	{
		return;
	}

	D3D11_TEXTURE2D_DESC tDesc;
	resultTex->GetDesc(&tDesc);

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	
	dc->Map(resultTex, 0, D3D11_MAP_READ, 0, &mappedRes);
	UINT* data = reinterpret_cast<UINT*>(mappedRes.pData);

	SaveBMP(filePath, data, tDesc.Width, tDesc.Height, mappedRes.RowPitch);

	dc->Unmap(resultTex, 0);
}
 
void LightsOutSaver::SaveBMP(std::wstring& filename, UINT* data, UINT width, UINT height, UINT rowPitch)
{
	HANDLE hFile;

	BITMAPFILEHEADER bmpFileHeader;
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024; 
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + sizeof(UINT) * width * height + height * (sizeof (UINT)* width) % 4;
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;

	BITMAPINFOHEADER bmpInfoHeader;
	ZeroMemory(&bmpInfoHeader, sizeof(BITMAPINFOHEADER));
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biBitCount = 32;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biWidth = width;
	bmpInfoHeader.biHeight = -1*height;
	bmpInfoHeader.biPlanes = 1;

	hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Cannot create file", L"Error", MB_OK);
		return;
	}

	BYTE Palette[1024];  
	memset(Palette, 0, 1024);

	DWORD writtenBytes;
	WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &writtenBytes, nullptr);
	WriteFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &writtenBytes, nullptr);

	WriteFile(hFile, Palette, 1024, &writtenBytes, nullptr);

	for(unsigned int i = 0; i < height; i++)
	{
		for(unsigned int j = 0; j < width; j++)
		{
			//ABGR to ARGB
			UINT color = data[i*rowPitch / sizeof(UINT) + j];
			BYTE A = (color >> 24) & 0xff; 
			BYTE B = (color >> 16) & 0xff;
			BYTE G = (color >>  8) & 0xff;
			BYTE R = (color >>  0) & 0xff;

			color = (A << 24) | (R << 16) | (G << 8) | (B << 0);
			WriteFile(hFile, &color, sizeof(UINT), &writtenBytes, nullptr);
		}

		WriteFile(hFile, Palette, (sizeof (UINT)* width) % 4, &writtenBytes, nullptr);
	}

	CloseHandle(hFile);
}