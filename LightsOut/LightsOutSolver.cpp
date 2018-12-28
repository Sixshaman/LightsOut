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
	mClickRule         = clickRule;
	mQuietPatternCount = 0;

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

		if(base[i].none())
		{
			mQuietPatternCount++;
		}
	}

	for(int i = 0; i < size_size; i++)
	{
		for(int j = 0; j < i; j++)
		{
			//std::swap doesn't work here
			bool temp = mInvSolutionMatrix[i][j];
			mInvSolutionMatrix[i].set(j, mInvSolutionMatrix[j][i]);
			mInvSolutionMatrix[j].set(i, temp);
		}
	}
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::GetSolution(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize, const LightsOutClickRule* clickRule)
{
	int si_si = gameSize * gameSize;

	boost::dynamic_bitset<uint32_t> solution;
	solution.resize(si_si, false);

	if(board.none())
	{
		return solution;
	}

	if(gameSize != mCurrentSize || mClickRule != clickRule)
	{
		mInvSolutionMatrix.clear();
		mCurrentSize = gameSize;
	}

	if(mInvSolutionMatrix.empty())
	{
		GenerateInverseMatrix(gameSize, clickRule);
	}

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

uint32_t LightsOutSolver::QuietPatternCount(uint16_t gameSize, const LightsOutClickRule* clickRule)
{
	if (gameSize != mCurrentSize || mClickRule != clickRule)
	{
		mInvSolutionMatrix.clear();
		mCurrentSize = gameSize;
	}

	if (mInvSolutionMatrix.empty())
	{
		GenerateInverseMatrix(gameSize, clickRule);
	}

	return mQuietPatternCount;
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::MoveLeft(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize)
{
	boost::dynamic_bitset<uint32_t> left;

	int si_si = gameSize * gameSize;
	left.resize(si_si, false);

	if(board.none())
	{
		return left;
	}

	for(int i = 0; i < board.size(); i++)
	{
		int32_t curX = i % gameSize;
		int32_t curY = i / gameSize;

		int32_t  leftX     = (((curX - 1) % gameSize) + gameSize) % gameSize;
		uint32_t leftIndex = curY * gameSize + leftX;

		left[leftIndex] = board[i];
	}

	return left;
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::MoveRight(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize)
{
	boost::dynamic_bitset<uint32_t> right;

	int si_si = gameSize * gameSize;
	right.resize(si_si, false);

	if (board.none())
	{
		return right;
	}

	for (int i = 0; i < board.size(); i++)
	{
		int32_t curX = i % gameSize;
		int32_t curY = i / gameSize;

		int32_t  rightX = (((curX + 1) % gameSize) + gameSize) % gameSize;
		uint32_t rightIndex = curY * gameSize + rightX;

		right[rightIndex] = board[i];
	}

	return right;
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::MoveUp(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize)
{
	boost::dynamic_bitset<uint32_t> up;

	int si_si = gameSize * gameSize;
	up.resize(si_si, false);

	if (board.none())
	{
		return up;
	}

	for (int i = 0; i < board.size(); i++)
	{
		int32_t curX = i % gameSize;
		int32_t curY = i / gameSize;

		int32_t  upY = (((curY - 1) % gameSize) + gameSize) % gameSize;
		uint32_t upIndex = upY * gameSize + curX;

		up[upIndex] = board[i];
	}

	return up;
}

boost::dynamic_bitset<uint32_t> LightsOutSolver::MoveDown(const boost::dynamic_bitset<uint32_t>& board, uint16_t gameSize)
{
	boost::dynamic_bitset<uint32_t> down;

	int si_si = gameSize * gameSize;
	down.resize(si_si, false);

	if (board.none())
	{
		return down;
	}

	for (int i = 0; i < board.size(); i++)
	{
		int32_t curX = i % gameSize;
		int32_t curY = i / gameSize;

		int32_t  downY     = (((curY + 1) % gameSize) + gameSize) % gameSize;
		uint32_t downIndex = downY * gameSize + curX;

		down[downIndex] = board[i];
	}

	return down;
}

