#ifndef LOSAVER_HPP
#define LOSAVER_HPP

#include <d3d11.h>
#include <string>

/*
* The saver class for Lights Out game boards.
*
*/

class LightsOutSaver
{
public:
	static void SaveBMP(std::wstring& filename, uint32_t* data, uint32_t width, uint32_t height, uint32_t rowPitch);
};

#endif LOSAVER_HPP