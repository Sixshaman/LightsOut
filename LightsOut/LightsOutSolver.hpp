#ifndef LIGHTS_OUT_SOLVER_HPP
#define LIGHTS_OUT_SOLVER_HPP

/*
 * The solver class for Lights Out game.
 *
 */

#include "LightsOutGame.hpp"
#include "LightsOutClickRules.hpp"

class LightsOutSolver
{
public:
	LightsOutSolver();
	~LightsOutSolver();

	boost::dynamic_bitset<uint32_t> GetSolution(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, const LightsOutClickRule* clickRule);
	boost::dynamic_bitset<uint32_t> GetInverseSolution(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, const LightsOutClickRule* clickRule);

	boost::dynamic_bitset<uint32_t> MoveLeft(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize);
	boost::dynamic_bitset<uint32_t> MoveRight(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize);
	boost::dynamic_bitset<uint32_t> MoveUp(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize);
	boost::dynamic_bitset<uint32_t> MoveDown(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize);

private:
	void GenerateInverseMatrix(uint16_t size, const LightsOutClickRule* clickRule);

private:
	LOMatrix                  mInvSolutionMatrix; //Current inverted matrix
	const LightsOutClickRule* mClickRule;         //Current click rule
	uint16_t                  mCurrentSize;       //Current game size
};

#endif