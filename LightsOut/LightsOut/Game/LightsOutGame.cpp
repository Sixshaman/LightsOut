#include "LightsOutGame.hpp"
#include "..\Util.hpp"

LightsOutGame::LightsOutGame(): mMainBoard(15, 2), mStability(15, 2)
{
	mClickRule.reset(new LightsOutClickRuleRegular(2));
	ResetStability(mMainBoard.Size());
}

LightsOutGame::~LightsOutGame()
{
}

void LightsOutGame::Reset(const LightsOutBoard& board, uint32_t resetFlags)
{
	if(resetFlags & RESET_FLAG_LEAVE_STABILITY)
	{
		mStability.BoardIncDif(mMainBoard, board);
	}
	else
	{
		ResetStability(board.Size());
	}

	mMainBoard = board;
}

void LightsOutGame::ResetStability(uint16_t gameSize)
{
	mStability.Reset(gameSize);
	mStability.Flip();
}

void LightsOutGame::Click(uint16_t posX, uint16_t posY)
{
	Clamp(posX, (uint16_t)0, (uint16_t)(mMainBoard.Size() - 1));
	Clamp(posY, (uint16_t)0, (uint16_t)(mMainBoard.Size() - 1));

	ResetStability(mMainBoard.Size());
	mClickRule->Click(&mMainBoard, posX, posY);
}

void LightsOutGame::ConstructClick(uint16_t posX, uint16_t posY)
{
	Clamp(posX, (uint16_t)0, (uint16_t)(mMainBoard.Size() - 1));
	Clamp(posY, (uint16_t)0, (uint16_t)(mMainBoard.Size() - 1));

	ResetStability(mMainBoard.Size());
	mMainBoard.IncrementCell(posX, posY);
}

void LightsOutGame::SetClickRuleRegular()
{
	mClickRule.reset(new LightsOutClickRuleRegular(mMainBoard.DomainSize()));
}

void LightsOutGame::SetClickRuleToroid()
{
	mClickRule.reset(new LightsOutClickRuleToroid(mMainBoard.DomainSize()));
}

void LightsOutGame::SetClickRuleBaked()
{
	mClickRule.reset(new LightsOutClickRuleCustom(mMainBoard));
}

void LightsOutGame::SetClickRuleBakedToroid()
{
	mClickRule.reset(new LightsOutClickRuleCustor(mMainBoard));
}

LightsOutBoard LightsOutGame::GetBoard()
{
	return mMainBoard;
}

LightsOutBoard LightsOutGame::GetStability()
{
	return mStability;
}

uint16_t LightsOutGame::GetSize()
{
	return mMainBoard.Size();
}

const LightsOutClickRule* LightsOutGame::GetClickRule() const
{
	return mClickRule.get();
}