#ifndef LIGHTS_OUT_SOLVER_HPP
#define LIGHTS_OUT_SOLVER_HPP

/*
 * The solver class for Lights Out game.
 *
 */

#include "..\Game\LightsOutGame.hpp"
#include "LightsOutClickRules.hpp"
#include "LightsOutMatrix.hpp"

class LightsOutSolver
{
public:
	LightsOutSolver();
	~LightsOutSolver();

	LightsOutBoard GetSolution(const LightsOutBoard& board, const LightsOutClickRule* clickRule);
	LightsOutBoard GetInverseSolution(const LightsOutBoard& board, const LightsOutClickRule* clickRule);

	uint32_t QuietPatternCount(uint16_t gameSize, uint16_t domainSize, const LightsOutClickRule* clickRule);

	LightsOutBoard MoveLeft(const LightsOutBoard& board);
	LightsOutBoard MoveRight(const LightsOutBoard& board);
	LightsOutBoard MoveUp(const LightsOutBoard& board);
	LightsOutBoard MoveDown(const LightsOutBoard& board);

private:
	void GenerateInverseMatrix(uint16_t size, const LightsOutClickRule* clickRule);

private:
	LightsOutMatrix           mInvSolutionMatrix; //Current inverted matrix
	const LightsOutClickRule* mClickRule;         //Current click rule
	uint32_t                  mQuietPatternCount; //Base number of quiet patterns
};

#endif