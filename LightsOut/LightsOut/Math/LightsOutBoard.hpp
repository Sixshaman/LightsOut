#pragma once

#include <cstdint>

class LightsOutBoardImpl;

class LightsOutBoard
{
public:
	LightsOutBoard(uint16_t gameSize, uint16_t domainSize);
	~LightsOutBoard();

	LightsOutBoard(const LightsOutBoard& right);
	LightsOutBoard operator =(const LightsOutBoard& right);

	bool operator==(const LightsOutBoard& right);

	uint32_t CellCount() const;
	uint16_t Size()      const;

	uint16_t DomainSize() const;

	void Reset(uint16_t gameSize);

	void     SetCellValue(uint16_t x, uint16_t y, uint16_t value);
	uint16_t GetCellValue(uint16_t x, uint16_t y) const;

	void IncrementCell(uint16_t x, uint16_t y);

	void BoardAdd(const LightsOutBoard& board);
	void BoardSub(const LightsOutBoard& board);
	void BoardMul(const LightsOutBoard& board);

	void BoardMulNum(uint16_t mul);
	void BoardSubMul(const LightsOutBoard& board,  uint16_t mul);
	void BoardIncDif(const LightsOutBoard& board1, const LightsOutBoard& board2);

	void DomainRotate();
	void DomainRotateNonZero();
	void InvertValues();

	bool IsNone() const;

	void CopyMemoryData(uint32_t* buf, uint32_t maxSize) const;

private:
	LightsOutBoardImpl* mpImpl;
};