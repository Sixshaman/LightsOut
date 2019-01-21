#pragma once

#include "..\Math\LightsOutBoard.hpp"
#include <vector>

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
	void Reset(const LightsOutBoard& solution);

	bool TurnsLeft() const { return !mTurns.empty(); }

	PointOnBoard GetRandomTurn();
	PointOnBoard GetFirstTurn();

private:
	std::vector<PointOnBoard> mTurns;
};