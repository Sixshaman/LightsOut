#include "LightsOutSolver.hpp"
#include "Util.hpp"

LightsOutSolver::LightsOutSolver(): mCurrentSize(0)
{
	mClickRule = nullptr;
}

LightsOutSolver::~LightsOutSolver()
{
}

void LightsOutSolver::GenerateInverseMatrix(uint16_t size, const LightsOutClickRule* clickRule)
{
	mClickRule = clickRule;

	int size_size = size*size;
	LOMatrix base = clickRule->GenerateGameMatrix(size);

	mInvSolutionMatrix.clear();

	for(int i = 0; i < size_size; i++)
	{
		mInvSolutionMatrix.push_back(boost::dynamic_bitset<uint32_t>(size_size));
		mInvSolutionMatrix[i].set(i);
	}

	for(int i = 0; i < size_size; i++)
	{
		if(!base[i][i])
		{
			for(int j = i + 1; j < size_size; j++)
			{
				if(base[j][i])
				{
					swap(base[i], base[j]);
					swap(mInvSolutionMatrix[i], mInvSolutionMatrix[j]);
					break;
				}
			}
		}

		for(int j = i + 1; j < size_size; j++)
		{
			if(base[j][i])
			{
				base[j] ^= base[i];
				mInvSolutionMatrix[j] ^= mInvSolutionMatrix[i];
			}
		}
	}

	for(int i = size_size - 1; i >= 0; i--)
	{
		for(int j = i - 1; j >= 0; j--)
		{
			if(base[j][i])
			{
				base[j] ^= base[i];
				mInvSolutionMatrix[j] ^= mInvSolutionMatrix[i];
			}
		}
	}
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::GetSolution(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, const LightsOutClickRule* clickRule)
{
	boost::dynamic_bitset<uint32_t> solution;

	if(board.none())
	{
		return solution;
	}

	if (gameSize != mCurrentSize || mClickRule != clickRule)
	{
		mInvSolutionMatrix.clear();
		mCurrentSize = gameSize;
	}

	if(mInvSolutionMatrix.empty())
	{
		GenerateInverseMatrix(gameSize, clickRule);
	}

	int si_si = gameSize * gameSize;
	solution.resize(si_si, false);

	//Calculate the solution by multiplying inverse matrix by board vector
	for (int i = 0; i < si_si; i++)
	{
		solution[i] = (mInvSolutionMatrix[i] & board).count() & 0x01; //One if dot product of matrix row and board vector is odd, zero otherwise.
	}

	return solution;
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::GetInverseSolution(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, const LightsOutClickRule* clickRule)
{
	boost::dynamic_bitset<uint32_t> inverseSolution;

	int si_si = gameSize * gameSize;
	inverseSolution.resize(si_si, false);

	if(board.none())
	{
		return inverseSolution;
	}

	for(int i = 0; i < board.size(); i++)
	{
		if(board[i])
		{
			uint16_t x = i % gameSize;
			uint16_t y = i / gameSize;

			clickRule->Click(inverseSolution, gameSize, x, y);
		}
	}

	return inverseSolution;
}

