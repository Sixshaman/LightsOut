#include "LOSaver.hpp"
#include "..\Graphics\LOTextures.hpp"
#include "..\Util.hpp"
#include "..\PNG\PNGSaver.hpp"
#include <memory>

void LightsOutSaver::SaveBMP(const std::wstring& filename, uint32_t* data, uint32_t width, uint32_t height, uint32_t rowPitch)
{
	HANDLE hFile;

	BITMAPFILEHEADER bmpFileHeader;
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024; 
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + sizeof(uint32_t) * width * height + height * (sizeof (uint32_t)* width) % 4;
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
			uint32_t color = data[i*rowPitch / sizeof(uint32_t) + j];
			BYTE A = (color >> 24) & 0xff; 
			BYTE B = (color >> 16) & 0xff;
			BYTE G = (color >>  8) & 0xff;
			BYTE R = (color >>  0) & 0xff;

			color = (A << 24) | (R << 16) | (G << 8) | (B << 0);
			WriteFile(hFile, &color, sizeof(uint32_t), &writtenBytes, nullptr);
		}

		WriteFile(hFile, Palette, (sizeof (uint32_t)* width) % 4, &writtenBytes, nullptr);
	}

	CloseHandle(hFile);
}

void LightsOutSaver::SavePNG(const std::wstring& filename, uint32_t* data, uint32_t width, uint32_t height, uint32_t rowPitch)
{
	PngSaver pngSaver;
	pngSaver.SavePngImage(filename, width, height, rowPitch, data);
}
