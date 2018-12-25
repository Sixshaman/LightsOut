#pragma once

#include <boost\dynamic_bitset.hpp>

typedef std::vector<boost::dynamic_bitset<uint32_t>> LOMatrix;

class LightsOutClickRule
{
public:
	virtual void     Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const = 0;
	virtual LOMatrix GenerateGameMatrix(uint16_t gameSize) const = 0;
};

//--------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleRegular: public LightsOutClickRule
{
public:
	LightsOutClickRuleRegular();
	~LightsOutClickRuleRegular();

	void Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const override;
	LOMatrix GenerateGameMatrix(uint16_t gameSize) const override;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleToroid: public LightsOutClickRule
{
public:
	LightsOutClickRuleToroid();
	~LightsOutClickRuleToroid();

	void Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const override;
	LOMatrix GenerateGameMatrix(uint16_t gameSize) const override;
};