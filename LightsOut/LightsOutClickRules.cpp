#include "LightsOutClickRules.hpp"

LightsOutClickRuleRegular::LightsOutClickRuleRegular()
{
}

LightsOutClickRuleRegular::~LightsOutClickRuleRegular()
{
}

void LightsOutClickRuleRegular::Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const
{
	board[posY*gameSize + posX].flip();
	if (posX > 0)            board[posY*gameSize + posX - 1].flip();
	if (posX < gameSize - 1) board[posY*gameSize + posX + 1].flip();
	if (posY > 0)            board[(posY - 1)*gameSize + posX].flip();
	if (posY < gameSize - 1) board[(posY + 1)*gameSize + posX].flip();
}

LOMatrix LightsOutClickRuleRegular::GenerateGameMatrix(uint16_t gameSize) const
{
	int si_si = gameSize * gameSize;
	LOMatrix lightMatrix;

	for (int i = 0; i < si_si; i++)
	{
		lightMatrix.push_back(boost::dynamic_bitset<uint32_t>(si_si));
		lightMatrix[i].reset();
	}

	for (int i = 0; i < si_si; i++)
	{
		lightMatrix[i].set(i);

		if (i >= gameSize)		  lightMatrix[i].set(i - gameSize);
		if (i < si_si - gameSize) lightMatrix[i].set(i + gameSize);

		if (i % gameSize) lightMatrix[i].set(i - 1);
		if ((i + 1) % gameSize) lightMatrix[i].set(i + 1);
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleRegular::RuleType() const
{
	return ClickRuleType::RULE_REGULAR;
}

//===========================================================================================================

LightsOutClickRuleToroid::LightsOutClickRuleToroid()
{
}

LightsOutClickRuleToroid::~LightsOutClickRuleToroid()
{
}

void LightsOutClickRuleToroid::Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const
{
	uint16_t left   = ((((int32_t)posX - 1) % gameSize) + gameSize) % gameSize;
	uint16_t right  = ((((int32_t)posX + 1) % gameSize) + gameSize) % gameSize;
	uint16_t top    = ((((int32_t)posY - 1) % gameSize) + gameSize) % gameSize;
	uint16_t bottom = ((((int32_t)posY + 1) % gameSize) + gameSize) % gameSize;

	board[posY*gameSize   + posX ].flip();
	board[posY*gameSize   + left ].flip();
	board[posY*gameSize   + right].flip();
	board[top*gameSize    + posX ].flip();
	board[bottom*gameSize + posX ].flip();
}

LOMatrix LightsOutClickRuleToroid::GenerateGameMatrix(uint16_t gameSize) const
{
	int si_si = gameSize * gameSize;
	LOMatrix lightMatrix;

	for (int i = 0; i < si_si; i++)
	{
		lightMatrix.push_back(boost::dynamic_bitset<uint32_t>(si_si));
		lightMatrix[i].reset();
	}

	for (int i = 0; i < si_si; i++)
	{
		int32_t iBig = i / gameSize;
		int32_t iSm  = i % gameSize;

		int32_t rightSm = (((iSm + 1)  % gameSize) + gameSize) % gameSize;
		int32_t leftSm  = (((iSm - 1)  % gameSize) + gameSize) % gameSize;
		int32_t topBig  = (((iBig - 1) % gameSize) + gameSize) % gameSize;
		int32_t botBig  = (((iBig + 1) % gameSize) + gameSize) % gameSize;

		uint16_t top   = topBig * gameSize + iSm;
		uint16_t bot   = botBig * gameSize + iSm;
		uint16_t left  = iBig   * gameSize + leftSm;
		uint16_t right = iBig   * gameSize + rightSm;

		lightMatrix[i].set(i);
		lightMatrix[i].set(top);
		lightMatrix[i].set(bot);
		lightMatrix[i].set(left);
		lightMatrix[i].set(right);
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleToroid::RuleType() const
{
	return ClickRuleType::RULE_TOROID;
}

//==================================================================================================================================

LightsOutClickRuleCustom::LightsOutClickRuleCustom(const boost::dynamic_bitset<uint32_t>& bakedRule, uint16_t ruleSize)
{
	mRuleDefinition = bakedRule;
	mRuleSize = ruleSize;
}

LightsOutClickRuleCustom::~LightsOutClickRuleCustom()
{
}

void LightsOutClickRuleCustom::Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const
{
	boost::dynamic_bitset<uint32_t> populatedRule;
	populatedRule.resize(board.size(), false);

	int32_t sizeHalf = mRuleSize / 2;

	int32_t left = (int32_t)posX - sizeHalf;
	int32_t top  = (int32_t)posY - sizeHalf;

	for(int32_t y = 0; y < mRuleSize; y++)
	{
		int32_t yBig = y + top;
		if (yBig < 0 || yBig >= gameSize)
		{
			continue;
		}

		for(int32_t x = 0; x < mRuleSize; x++)
		{
			int32_t xBig = x + left;
			if (xBig < 0 || xBig >= gameSize)
			{
				continue;
			}

			uint32_t indexBig       = yBig * gameSize + xBig;
			uint32_t indexSm        = y * mRuleSize + x;
			populatedRule[indexBig] = mRuleDefinition[indexSm];
		}
	}

	board = board ^ populatedRule;
}

LOMatrix LightsOutClickRuleCustom::GenerateGameMatrix(uint16_t gameSize) const
{
	int si_si = gameSize * gameSize;
	LOMatrix lightMatrix;

	for(int i = 0; i < si_si; i++)
	{
		lightMatrix.push_back(boost::dynamic_bitset<uint32_t>(si_si));
		lightMatrix[i].reset();
	}

	for(int i = 0; i < si_si; i++)
	{
		uint16_t x = i % gameSize;
		uint16_t y = i / gameSize;

		Click(lightMatrix[i], gameSize, x, y);
	}

	return lightMatrix;
}

ClickRuleType LightsOutClickRuleCustom::RuleType() const
{
	return ClickRuleType::RULE_CUSTOM;
}

uint16_t LightsOutClickRuleCustom::RuleSize() const
{
	return mRuleSize;
}
