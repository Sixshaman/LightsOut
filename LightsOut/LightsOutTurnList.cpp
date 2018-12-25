#include "LightsOutTurnList.hpp"
#include <random>

LightsOutTurnList::LightsOutTurnList()
{
}

LightsOutTurnList::~LightsOutTurnList()
{
}

void LightsOutTurnList::Reset(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize)
{
	for (int i = 0; i < board.size(); i++)
	{
		if (board[i])
		{
			mTurns.push_back(PointOnBoard((unsigned short)(i % gameSize), (unsigned short)(i / gameSize)));
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