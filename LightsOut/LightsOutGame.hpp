#ifndef LIGHTS_OUT_GEN_HPP
#define LIGHTS_OUT_GEN_HPP

#include <boost\dynamic_bitset\dynamic_bitset.hpp>
#include <vector>
#include <Windows.h>
#include <memory>
#include "LightsOutClickRules.hpp"

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

	void Reset(uint16_t size, const boost::dynamic_bitset<uint32_t>& board, uint32_t resetFlags);

	boost::dynamic_bitset<uint32_t> GetBoard();
	boost::dynamic_bitset<uint32_t> GetStability();
	uint16_t                        GetSize();

	const LightsOutClickRule* GetClickRule() const;

private:
	void ResetStability();

private:
	boost::dynamic_bitset<uint32_t> mMainBoard;
	boost::dynamic_bitset<uint32_t> mStability;
	uint16_t mSize;

	std::unique_ptr<LightsOutClickRule> mClickRule;
};

#endif