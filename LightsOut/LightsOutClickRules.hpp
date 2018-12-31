#pragma once

#include <boost\dynamic_bitset.hpp>

typedef std::vector<boost::dynamic_bitset<uint32_t>> LOMatrix;

enum class ClickRuleType
{
	RULE_REGULAR,
	RULE_TOROID,
	RULE_CUSTOM,
	RULE_CUSTOR
};

class LightsOutClickRule
{
public:
	virtual void          Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const = 0;
	virtual LOMatrix      GenerateGameMatrix(uint16_t gameSize) const = 0;
	virtual ClickRuleType RuleType() const = 0;
};

//--------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleRegular: public LightsOutClickRule
{
public:
	LightsOutClickRuleRegular();
	~LightsOutClickRuleRegular();

	void Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const override;
	LOMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleToroid: public LightsOutClickRule
{
public:
	LightsOutClickRuleToroid();
	~LightsOutClickRuleToroid();

	void Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const override;
	LOMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleCustom: public LightsOutClickRule
{
public:
	LightsOutClickRuleCustom(const boost::dynamic_bitset<uint32_t>& bakedRule, uint16_t ruleSize);
	~LightsOutClickRuleCustom();

	void Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const override;
	LOMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;

	uint16_t RuleSize() const;

private:
	boost::dynamic_bitset<uint32_t> mRuleDefinition;
	uint16_t mRuleSize;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleCustor: public LightsOutClickRule
{
public:
	LightsOutClickRuleCustor(const boost::dynamic_bitset<uint32_t>& bakedRule, uint16_t ruleSize);
	~LightsOutClickRuleCustor();

	void Click(boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, uint16_t posX, uint16_t posY) const override;
	LOMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;

	uint16_t RuleSize() const;

private:
	boost::dynamic_bitset<uint32_t> mRuleDefinition;
	uint16_t mRuleSize;
};