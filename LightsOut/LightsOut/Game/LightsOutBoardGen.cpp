#include "LightsOutBoardGen.hpp"
#include <random>

LightsOutBoardGen::LightsOutBoardGen()
{
}

LightsOutBoardGen::~LightsOutBoardGen()
{
}

LightsOutBoard LightsOutBoardGen::Generate(uint16_t gameSize, uint16_t domainSize, BoardGenMode mode)
{
	LightsOutBoard result(gameSize, domainSize);

	uint16_t minLo = 0;
	uint16_t maxLo = domainSize - 1;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> loDistrib(minLo, maxLo);

	for(uint16_t y = 0; y < gameSize; y++)
	{
		for (uint16_t x = 0; x < gameSize; x++)
		{
			switch (mode)
			{
			case BoardGenMode::BOARDGEN_FULL_RANDOM:
			{
				result.SetCellValue(x, y, loDistrib(gen));
				break;
			}
			case BoardGenMode::BOARDGEN_ZERO_ELEMENT:
			{
				result.SetCellValue(x, y, minLo);
				break;
			}
			case BoardGenMode::BOARDGEN_ONE_ELEMENT:
			{
				result.SetCellValue(x, y, maxLo);
				break;
			}
			case BoardGenMode::BOARDGEN_BLATNOY:
			{
				if (x % 2 == y % 2)
				{
					result.SetCellValue(x, y, maxLo);
				}
				else
				{
					result.SetCellValue(x, y, minLo);
				}
				break;
			}
			case BoardGenMode::BOARDGEN_PETYA_STYLE:
			{
				if (y % 2)
				{
					if (!(x % 2))
					{
						result.SetCellValue(x, y, maxLo);
					}
					else
					{
						result.SetCellValue(x, y, minLo);
					}
				}
				else
				{
					result.SetCellValue(x, y, minLo);
				}
				break;
			}
			case BoardGenMode::BOARDGEN_BORDER:
			{
				if(y == 0 || y == (gameSize - 1) || x == 0 || x == (gameSize - 1))
				{
					result.SetCellValue(x, y, maxLo);
				}
				else
				{
					result.SetCellValue(x, y, minLo);
				}
				break;
			}
			}
		}
	}

	return result;
}
