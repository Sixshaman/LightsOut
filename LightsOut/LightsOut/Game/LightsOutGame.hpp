#ifndef LIGHTS_OUT_GEN_HPP
#define LIGHTS_OUT_GEN_HPP

#include <vector>
#include <Windows.h>
#include <memory>
#include "..\Math\LightsOutClickRules.hpp"
#include "..\Math\LightsOutBoard.hpp"

/*
* The Lights Out game class.
*
*/

#define RESET_FLAG_LEAVE_STABILITY 1

class LightsOutGame
{
public:
	LightsOutGame();
	~LightsOutGame();

	void Click(uint16_t xPos, uint16_t yPos);
	void ConstructClick(uint16_t xPos, uint16_t yPos);

	void SetClickRuleRegular();
	void SetClickRuleToroid();
	void SetClickRuleBaked();
	void SetClickRuleBakedToroid();

	void Reset(const LightsOutBoard& board, uint32_t resetFlags);

	LightsOutBoard GetBoard();
	LightsOutBoard GetStability();
	uint16_t       GetSize();

	const LightsOutClickRule* GetClickRule() const;

private:
	void ResetStability(uint16_t gameSize);

private:
	LightsOutBoard mMainBoard;
	LightsOutBoard mStability;

	std::unique_ptr<LightsOutClickRule> mClickRule;
};

#endif