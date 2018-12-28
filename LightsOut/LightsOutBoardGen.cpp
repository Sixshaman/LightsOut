#include "LightsOutBoardGen.hpp"
#include <random>

LightsOutBoardGen::LightsOutBoardGen()
{
}

LightsOutBoardGen::~LightsOutBoardGen()
{
}

boost::dynamic_bitset<uint32_t> LightsOutBoardGen::Generate(unsigned short size, BoardGenMode mode)
{
	uint32_t si_si = size * size;
	boost::dynamic_bitset<uint32_t> board(si_si);

	std::random_device rd;
	for (uint16_t i = 0; i < si_si; i++)
	{
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> booleanDistrib(0, 1);

		switch (mode)
		{
		case BoardGenMode::BOARDGEN_FULL_RANDOM:
			board.set(i, booleanDistrib(gen) == 1);
			break;
		case BoardGenMode::BOARDGEN_ZERO_ELEMENT:
			board.set(i, false);
			break;
		case BoardGenMode::BOARDGEN_ONE_ELEMENT:
			board.set(i, true);
			break;
		case BoardGenMode::BOARDGEN_BLATNOY:
			if ((i / size) % 2 == (i % size) % 2)
			{
				board.set(i, 1);
			}
			else
			{
				board.set(i, 0);
			}
			break;
		case BoardGenMode::BOARDGEN_PETYA_STYLE:
			if ((i / size) % 2)
			{
				if (!((i % size) % 2))
					board.set(i, 1);
				else
					board.set(i, 0);
			}
			else
			{
				board.set(i, 0);
			}
			break;
		case BoardGenMode::BOARDGEN_BORDER:
			if (i / size == 0 || i / size == (size - 1) || i % size == 0 || i % size == (size - 1))
			{
				board.set(i, 1);
			}
			else
			{
				board.set(i, 0);
			}
			break;
		}
	}

	return board;
}
