#pragma once

#include <boost\dynamic_bitset.hpp>

struct PointOnBoard
{
	uint16_t boardX;
	uint16_t boardY;

	PointOnBoard(unsigned short _x, unsigned short _y) : boardX(_x), boardY(_y) {}
	bool operator ==(PointOnBoard T) { return (boardX == T.boardX && boardY == T.boardY); };
};

class LightsOutTurnList
{
public:
	LightsOutTurnList();
	~LightsOutTurnList();

	void Clear();
	void Reset(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize);

	bool TurnsLeft() const { return !mTurns.empty(); }

	PointOnBoard GetRandomTurn();
	PointOnBoard GetFirstTurn();

private:
	std::vector<PointOnBoard> mTurns;
};