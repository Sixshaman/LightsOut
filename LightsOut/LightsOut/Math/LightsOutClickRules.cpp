#include "LightsOutClickRules.hpp"

LightsOutClickRuleRegular::LightsOutClickRuleRegular(uint16_t domainSize): mDomainSize(domainSize)
{
}

LightsOutClickRuleRegular::~LightsOutClickRuleRegular()
{
}

void LightsOutClickRuleRegular::Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const
{
	board->IncrementCell(posX, posY);

	if(posX > 0)
	{
		board->IncrementCell(posX - 1, posY);
	}

	if(posX < board->Size() - 1)
	{
		board->IncrementCell(posX + 1, posY);
	}

	if(posY > 0)
	{
		board->IncrementCell(posX, posY - 1);
	}

	if(posY < board->Size() - 1)
	{
		board->IncrementCell(posX, posY + 1);
	}
}

LightsOutMatrix LightsOutClickRuleRegular::GenerateGameMatrix(uint16_t gameSize) const
{
	LightsOutMatrix lightMatrix(gameSize, mDomainSize);
	for(uint16_t y = 0; y < gameSize; y++)
	{
		for(uint16_t x = 0; x < gameSize; x++)
		{
			lightMatrix.SetVal(x, y, x, y, 1);

			if(x > 0)
			{
				lightMatrix.SetVal(x, y, x - 1, y, 1);
			}

			if(x < gameSize - 1)
			{
				lightMatrix.SetVal(x, y, x + 1, y, 1);
			}

			if(y > 0)
			{
				lightMatrix.SetVal(x, y, x, y - 1, 1);
			}

			if(y < gameSize - 1)
			{
				lightMatrix.SetVal(x, y, x, y + 1, 1);
			}
		}
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleRegular::RuleType() const
{
	return ClickRuleType::RULE_REGULAR;
}

//===========================================================================================================

LightsOutClickRuleToroid::LightsOutClickRuleToroid(uint16_t domainSize): mDomainSize(domainSize)
{
}

LightsOutClickRuleToroid::~LightsOutClickRuleToroid()
{
}

void LightsOutClickRuleToroid::Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const
{
	uint16_t gameSize = board->Size();

	uint16_t left   = ((((int32_t)posX - 1) % gameSize) + gameSize) % gameSize;
	uint16_t right  = ((((int32_t)posX + 1) % gameSize) + gameSize) % gameSize;
	uint16_t top    = ((((int32_t)posY - 1) % gameSize) + gameSize) % gameSize;
	uint16_t bottom = ((((int32_t)posY + 1) % gameSize) + gameSize) % gameSize;

	board->IncrementCell(posX,  posY);
	board->IncrementCell(left,  posY);
	board->IncrementCell(right, posY);
	board->IncrementCell(posX,  top);
	board->IncrementCell(posX,  bottom);
}

LightsOutMatrix LightsOutClickRuleToroid::GenerateGameMatrix(uint16_t gameSize) const
{
	LightsOutMatrix lightMatrix(gameSize, mDomainSize);
	for(int32_t y = 0; y < gameSize; y++)
	{
		for(int32_t x = 0; x < gameSize; x++)
		{
			int32_t left   = (int32_t)((((x - 1) % gameSize) + gameSize) % gameSize);
			int32_t right  = (int32_t)((((x + 1) % gameSize) + gameSize) % gameSize);
			int32_t top    = (int32_t)((((y - 1) % gameSize) + gameSize) % gameSize);
			int32_t bottom = (int32_t)((((y + 1) % gameSize) + gameSize) % gameSize);

			lightMatrix.SetVal(x, y,     x,      y, 1);
			lightMatrix.SetVal(x, y,  left,      y, 1);
			lightMatrix.SetVal(x, y, right,      y, 1);
			lightMatrix.SetVal(x, y,     x,    top, 1);
			lightMatrix.SetVal(x, y,     x, bottom, 1);
		}
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleToroid::RuleType() const
{
	return ClickRuleType::RULE_TOROID;
}

//==================================================================================================================================

LightsOutClickRuleCustom::LightsOutClickRuleCustom(const LightsOutBoard& bakedRule): mRuleDefinition(bakedRule)
{
	mRuleDefinition = bakedRule;
}

LightsOutClickRuleCustom::~LightsOutClickRuleCustom()
{
}

void LightsOutClickRuleCustom::Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const
{
	LightsOutBoard populatedRule(board->Size(), board->DomainSize());

	int32_t boardSize = board->Size();
	int32_t ruleSize  = mRuleDefinition.Size();
	int32_t sizeHalf  = ruleSize / 2;

	int32_t left = (int32_t)posX - sizeHalf;
	int32_t top  = (int32_t)posY - sizeHalf;

	for(int32_t y = 0; y < ruleSize; y++)
	{
		int32_t yBig = y + top;
		if(yBig < 0 || yBig >= boardSize)
		{
			continue;
		}

		for(int32_t x = 0; x < ruleSize; x++)
		{
			int32_t xBig = x + left;
			if(xBig < 0 || xBig >= boardSize)
			{
				continue;
			}

			populatedRule.SetCellValue(xBig, yBig, mRuleDefinition.GetCellValue(x, y));
		}
	}

	board->BoardAdd(populatedRule);
}

LightsOutMatrix LightsOutClickRuleCustom::GenerateGameMatrix(uint16_t gameSize) const
{
	LightsOutMatrix lightMatrix(gameSize, mRuleDefinition.DomainSize());
	for (uint16_t y = 0; y < gameSize; y++)
	{
		for (uint16_t x = 0; x < gameSize; x++)
		{
			LightsOutBoard board = lightMatrix.GetCellClickRule(x, y);
			Click(&board, x, y);
			lightMatrix.SetCellClickRule(x, y, board);
		}
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleCustom::RuleType() const
{
	return ClickRuleType::RULE_CUSTOM;
}

uint16_t LightsOutClickRuleCustom::RuleSize() const
{
	return mRuleDefinition.Size();
}

//==================================================================================================================================

LightsOutClickRuleCustor::LightsOutClickRuleCustor(const LightsOutBoard& bakedRule): mRuleDefinition(bakedRule)
{
	mRuleDefinition = bakedRule;
}

LightsOutClickRuleCustor::~LightsOutClickRuleCustor()
{
}

void LightsOutClickRuleCustor::Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const
{
	LightsOutBoard populatedRule(board->Size(), board->DomainSize());

	int32_t boardSize = board->Size();
	int32_t ruleSize  = mRuleDefinition.Size();
	int32_t sizeHalf  = ruleSize / 2;

	int32_t left = (int32_t)posX - sizeHalf;
	int32_t top  = (int32_t)posY - sizeHalf;

	for(int32_t y = 0; y < ruleSize; y++)
	{
		int32_t yBig    = y + top;
		int32_t yBigMod = ((yBig % boardSize) + boardSize) % boardSize;

		for(int32_t x = 0; x < ruleSize; x++)
		{
			int32_t xBig    = x + left;
			int32_t xBigMod = ((xBig % boardSize) + boardSize) % boardSize;

			uint16_t cumulRuleValue = populatedRule.GetCellValue(xBigMod, yBigMod);
			uint16_t ruleValue      = mRuleDefinition.GetCellValue(x, y);
			populatedRule.SetCellValue(xBigMod, yBigMod, (cumulRuleValue + ruleValue) % board->DomainSize());
		}
	}

	board->BoardAdd(populatedRule);
}

LightsOutMatrix LightsOutClickRuleCustor::GenerateGameMatrix(uint16_t gameSize) const
{
	LightsOutMatrix lightMatrix(gameSize, mRuleDefinition.DomainSize());
	int si_si = lightMatrix.GetMatrixSize();

	for(int i = 0; i < si_si; i++)
	{
		uint16_t x = i % gameSize;
		uint16_t y = i / gameSize;

		LightsOutBoard board = lightMatrix.GetCellClickRule(x, y);
		Click(&board, x, y);
		lightMatrix.SetCellClickRule(x, y, board);
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleCustor::RuleType() const
{
	return ClickRuleType::RULE_CUSTOR;
}

uint16_t LightsOutClickRuleCustor::RuleSize() const
{
	return mRuleDefinition.Size();
}
