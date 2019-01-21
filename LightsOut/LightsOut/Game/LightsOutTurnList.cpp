#include "LightsOutTurnList.hpp"
#include <random>

LightsOutTurnList::LightsOutTurnList()
{
}

LightsOutTurnList::~LightsOutTurnList()
{
}

void LightsOutTurnList::Clear()
{
	mTurns.clear();
}

void LightsOutTurnList::Reset(const LightsOutBoard& solution)
{
	for(uint16_t y = 0; y < solution.Size(); y++)
	{
		for(uint16_t x = 0; x < solution.Size(); x++)
		{
			uint16_t cellVal = solution.GetCellValue(x, y);
			for(uint16_t i = 0; i < cellVal; i++)
			{
				mTurns.push_back(PointOnBoard(x, y));
			}
		}
	}
}

PointOnBoard LightsOutTurnList::GetRandomTurn()
{
	if (!TurnsLeft())
	{
		return PointOnBoard((uint16_t)-1, (uint16_t)-1);
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> Distrib(0, mTurns.size() - 1);

	auto randomIterator = mTurns.begin() + Distrib(rd);

	PointOnBoard randomTurn = *randomIterator;
	mTurns.erase(randomIterator);

	return randomTurn;
}

PointOnBoard LightsOutTurnList::GetFirstTurn()
{
	if (!TurnsLeft())
	{
		return PointOnBoard((uint16_t)-1, (uint16_t)-1);
	}

	auto firstIterator = mTurns.begin();

	PointOnBoard firstTurn = *firstIterator;
	mTurns.erase(firstIterator);

	return firstTurn;
}