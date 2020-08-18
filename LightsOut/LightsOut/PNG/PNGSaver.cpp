#include "PNGSaver.hpp"
#include "FileHandle.hpp"

#ifdef USE_PNG_SAVER

RGBACOLOR::RGBACOLOR(): R(255), G(255), B(255), A(255)
{
}

PngSaver::PngSaver(): mPngStruct(nullptr), mPngInfo(nullptr)
{
	mPngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(mPngStruct)
	{
		mPngInfo = png_create_info_struct(mPngStruct);
	}

	if(!mPngInfo || !mPngStruct || setjmp(png_jmpbuf(mPngStruct)) != 0)
	{
		png_destroy_write_struct(&mPngStruct, &mPngInfo);
	}
}

PngSaver::~PngSaver()
{
	png_destroy_write_struct(&mPngStruct, &mPngInfo);
}

void PngSaver::SavePngImage(const std::wstring& filename, size_t width, size_t height, size_t rowPitch, uint32_t* data)
{
	if(width > rowPitch)
	{
		return;
	}

	std::vector<std::vector<png_byte>> imgRows(height);

	std::vector<png_bytep> rowPointers(height);
	for (size_t i = 0; i < height; i++)
	{
		std::vector<png_byte> row((size_t)width * 4);
		for (size_t j = 0; j < width; j++)
		{
			uint32_t color = data[i*rowPitch / sizeof(uint32_t) + j];
			png_byte A = (color >> 24) & 0xff;
			png_byte B = (color >> 16) & 0xff;
			png_byte G = (color >>  8) & 0xff;
			png_byte R = (color >>  0) & 0xff;

			row[j * 4 + 0] = R;
			row[j * 4 + 1] = G;
			row[j * 4 + 2] = B;
			row[j * 4 + 3] = A;
		}

		imgRows[i]     = row;
		rowPointers[i] = &imgRows[i][0];
	}

	FileHandle fout(filename, L"wb");
	if(!fout)
	{
		return;
	}

	png_init_io(mPngStruct, fout.GetFilePointer());

	png_set_IHDR(mPngStruct, mPngInfo,
		         (png_uint_32)width, (png_uint_32)height,
		         8,
		         PNG_COLOR_TYPE_RGBA,
		         PNG_INTERLACE_NONE,
		         PNG_COMPRESSION_TYPE_DEFAULT,
		         PNG_FILTER_TYPE_DEFAULT);

	png_write_info(mPngStruct, mPngInfo);

	png_write_image(mPngStruct, rowPointers.data());
	png_write_end(mPngStruct, nullptr);
}

bool PngSaver::operator!() const
{
	return mPngStruct == nullptr || mPngInfo == nullptr;
}

#endif