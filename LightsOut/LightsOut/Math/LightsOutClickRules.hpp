#pragma once

#include "LightsOutBoard.hpp"
#include "LightsOutMatrix.hpp"
#include <memory>

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
	virtual void            Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const = 0;
	virtual LightsOutMatrix GenerateGameMatrix(uint16_t gameSize) const = 0;
	virtual ClickRuleType   RuleType() const = 0;
};

//--------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleRegular: public LightsOutClickRule
{
public:
	LightsOutClickRuleRegular(uint16_t domainSize);
	~LightsOutClickRuleRegular();

	void Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const override;
	LightsOutMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;

private:
	uint16_t mDomainSize;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleToroid: public LightsOutClickRule
{
public:
	LightsOutClickRuleToroid(uint16_t domainSize);
	~LightsOutClickRuleToroid();

	void Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const override;
	LightsOutMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;

private:
	uint16_t mDomainSize;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleCustom: public LightsOutClickRule
{
public:
	LightsOutClickRuleCustom(const LightsOutBoard& bakedRule);
	~LightsOutClickRuleCustom();

	void Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const override;
	LightsOutMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;

	uint16_t RuleSize() const;

private:
	LightsOutBoard mRuleDefinition;
};

//---------------------------------------------------------------------------------------------------------------------------------

class LightsOutClickRuleCustor: public LightsOutClickRule
{
public:
	LightsOutClickRuleCustor(const LightsOutBoard& bakedRule);
	~LightsOutClickRuleCustor();

	void Click(LightsOutBoard* board, uint16_t posX, uint16_t posY) const override;
	LightsOutMatrix GenerateGameMatrix(uint16_t gameSize) const override;
	ClickRuleType RuleType() const override;

	uint16_t RuleSize() const;

private:
	LightsOutBoard mRuleDefinition;
};