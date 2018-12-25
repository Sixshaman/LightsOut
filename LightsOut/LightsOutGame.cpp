#include "LightsOutGame.hpp"
#include "Util.hpp"

LightsOutGame::LightsOutGame(): mSize(15)
{
	mClickRule.reset(new LightsOutClickRuleRegular());
}

LightsOutGame::~LightsOutGame()
{
}

void LightsOutGame::Reset(uint16_t size, const boost::dynamic_bitset<uint32_t>& board, uint32_t resetFlags)
{
	if(resetFlags & RESET_FLAG_LEAVE_STABILITY)
	{
		mStability = mStability & (~(mMainBoard ^ board));
	}
	else
	{
		ResetStability();
	}

	mSize = size;
	mMainBoard = board;
}

void LightsOutGame::ResetStability()
{
	if(mStability.size() != mSize * mSize)
	{
		mStability.resize(mSize * mSize, true);
	}

	mStability.reset();
	mStability = ~mStability;
}

void LightsOutGame::Click(uint16_t posX, uint16_t posY)
{
	Clamp(posX, (uint16_t)0, (uint16_t)(mSize - 1));
	Clamp(posY, (uint16_t)0, (uint16_t)(mSize - 1));

	ResetStability();
	mClickRule->Click(mMainBoard, mSize, posX, posY);
}

void LightsOutGame::ConstructClick(uint16_t posX, uint16_t posY)
{
	Clamp(posX, (uint16_t)0, (uint16_t)(mSize - 1));
	Clamp(posY, (uint16_t)0, (uint16_t)(mSize - 1));

	ResetStability();
	mMainBoard[posY*mSize + posX].flip();
}

void LightsOutGame::SetClickRuleRegular()
{
	mClickRule.reset(new LightsOutClickRuleRegular());
}

void LightsOutGame::SetClickRuleToroid()
{
	mClickRule.reset(new LightsOutClickRuleToroid());
}

boost::dynamic_bitset<uint32_t> LightsOutGame::GetBoard()
{
	return mMainBoard;
}

boost::dynamic_bitset<uint32_t> LightsOutGame::GetStability()
{
	return mStability;
}

uint16_t LightsOutGame::GetSize()
{
	return mSize;
}

const LightsOutClickRule* LightsOutGame::GetClickRule() const
{
	return mClickRule.get();
}