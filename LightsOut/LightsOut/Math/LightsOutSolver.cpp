#include "LightsOutSolver.hpp"
#include "..\Util.hpp"

LightsOutSolver::LightsOutSolver(): mClickRule(nullptr), mInvSolutionMatrix(0, 1)
{
}

LightsOutSolver::~LightsOutSolver()
{
}

void LightsOutSolver::GenerateInverseMatrix(uint16_t size, const LightsOutClickRule* clickRule)
{
	mClickRule         = clickRule;
	mQuietPatternCount = 0;

	mInvSolutionMatrix = clickRule->GenerateGameMatrix(size);
	mQuietPatternCount = mInvSolutionMatrix.Inverto();
}

LightsOutBoard LightsOutSolver::GetSolution(const LightsOutBoard& board, const LightsOutClickRule* clickRule)
{
	LightsOutBoard solution(board.Size(), board.DomainSize());
	if(board.IsNone())
	{
		return solution;
	}

	if(board.Size() != mInvSolutionMatrix.GetGameSize() || board.DomainSize() != mInvSolutionMatrix.GetDomainSize() || mClickRule != clickRule)
	{
		GenerateInverseMatrix(board.Size(), clickRule);
	}

	//Calculate the solution by multiplying inverse matrix by board vector
	for(uint16_t y = 0; y < board.Size(); y++)
	{
		for(uint16_t x = 0; x < board.Size(); x++)
		{
			LightsOutBoard cellSolution = mInvSolutionMatrix.GetCellClickRule(x, y);
			cellSolution.BoardMul(board);

			solution.SetCellValue(x, y, cellSolution.GetCellValue(0, 0));
		}
	}

	solution.InvertValues();
	return solution;
}

LightsOutBoard LightsOutSolver::GetInverseSolution(const LightsOutBoard& board, const LightsOutClickRule* clickRule)
{
	LightsOutBoard inverseSolution(board.Size(), board.DomainSize());
	if(board.IsNone())
	{
		return inverseSolution;
	}

	for(uint16_t y = 0; y < board.Size(); y++)
	{
		for (uint16_t x = 0; x < board.Size(); x++)
		{
			uint16_t cellValue = board.GetCellValue(x, y);
			for(uint16_t cl = 0; cl < cellValue; cl++)
			{
				clickRule->Click(&inverseSolution, x, y);
			}
		}
	}

	inverseSolution.InvertValues();
	return inverseSolution;
}

uint32_t LightsOutSolver::QuietPatternCount(uint16_t gameSize, uint16_t domainSize, const LightsOutClickRule* clickRule)
{
	if(gameSize != mInvSolutionMatrix.GetGameSize() || domainSize != mInvSolutionMatrix.GetDomainSize() || mClickRule != clickRule)
	{
		GenerateInverseMatrix(gameSize, clickRule);
	}

	return mQuietPatternCount;
}

LightsOutBoard LightsOutSolver::MoveLeft(const LightsOutBoard& board)
{
	LightsOutBoard left(board.Size(), board.DomainSize());
	if(board.IsNone())
	{
		return left;
	}

	for(int32_t y = 0; y < board.Size(); y++)
	{
		for (int32_t x = 0; x < board.Size(); x++)
		{
			int32_t leftX = (((x - 1) % board.Size()) + board.Size()) % board.Size();
			left.SetCellValue(leftX, y, board.GetCellValue(x, y));
		}
	}

	return left;
}

LightsOutBoard LightsOutSolver::MoveRight(const LightsOutBoard& board)
{
	LightsOutBoard right(board.Size(), board.DomainSize());
	if(board.IsNone())
	{
		return right;
	}
	
	for (int32_t y = 0; y < board.Size(); y++)
	{
		for (int32_t x = 0; x < board.Size(); x++)
		{
			int32_t rightX = (((x + 1) % board.Size()) + board.Size()) % board.Size();
			right.SetCellValue(rightX, y, board.GetCellValue(x, y));
		}
	}

	return right;
}

LightsOutBoard LightsOutSolver::MoveUp(const LightsOutBoard& board)
{
	LightsOutBoard up(board.Size(), board.DomainSize());
	if(board.IsNone())
	{
		return up;
	}

	for (int32_t y = 0; y < board.Size(); y++)
	{
		for (int32_t x = 0; x < board.Size(); x++)
		{
			int32_t upY = (((y - 1) % board.Size()) + board.Size()) % board.Size();
			up.SetCellValue(x, upY, board.GetCellValue(x, y));
		}
	}

	return up;
}

LightsOutBoard LightsOutSolver::MoveDown(const LightsOutBoard& board)
{
	LightsOutBoard down(board.Size(), board.DomainSize());
	if(board.IsNone())
	{
		return down;
	}

	for (int32_t y = 0; y < board.Size(); y++)
	{
		for (int32_t x = 0; x < board.Size(); x++)
		{
			int32_t downY = (((y + 1) % board.Size()) + board.Size()) % board.Size();
			down.SetCellValue(x, downY, board.GetCellValue(x, y));
		}
	}

	return down;
}

