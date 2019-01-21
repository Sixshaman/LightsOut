#pragma once

#include <vector>
#include "LightsOutBoard.hpp"

class LightsOutMatrix
{
public:
	LightsOutMatrix(uint16_t gameSize, uint16_t domainSize);
	~LightsOutMatrix();

	uint32_t GetGameSize();
	uint32_t GetMatrixSize();
	uint16_t GetDomainSize();

	void Resize(uint16_t gameSize);

	void     SetVal(uint16_t cellRuleX, uint16_t cellRuleY, uint16_t x, uint16_t y, uint16_t val);
	uint16_t GetVal(uint16_t cellRuleX, uint16_t cellRuleY, uint16_t x, uint16_t y);

	LightsOutBoard GetCellClickRule(uint16_t cellRuleX, uint16_t cellRuleY);
	void           SetCellClickRule(uint16_t cellRuleX, uint16_t cellRuleY, const LightsOutBoard& board);

	uint32_t Inverto();

private:
	std::vector<LightsOutBoard> mCellClickRules;

	uint32_t mSizeSmall;
	uint32_t mSizeBig;
	uint16_t mDomainSize;
};