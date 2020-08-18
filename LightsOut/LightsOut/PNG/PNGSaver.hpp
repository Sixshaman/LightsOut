#pragma once

#include <string>
#include <vector>

#ifdef USE_PNG_SAVER

#include <libpng16/png.h>

struct RGBACOLOR
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;

	RGBACOLOR();
};

class PngSaver //This class is needed to save PNG data to file
{
public:
	PngSaver();
	~PngSaver();

	PngSaver(const PngSaver&)           = delete;
	PngSaver operator=(const PngSaver&) = delete;

	PngSaver(const PngSaver&&)           = delete;
	PngSaver operator=(const PngSaver&&) = delete;

	void SavePngImage(const std::wstring& filename, size_t width, size_t height, size_t rowPitch, uint32_t* data);

	bool operator!() const;

private:
	png_structp mPngStruct;
	png_infop   mPngInfo;
};

#endif