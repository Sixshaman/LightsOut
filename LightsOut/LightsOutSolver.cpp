#include "LightsOutSolver.hpp"
#include <algorithm>
#include <random>
#include "Util.hpp"

Solver::Solver(): mSolvingFlag(false), mSolution(boost::dynamic_bitset<uint32_t>()), mSolvingInverted(false), mCurrentSize(0)
{
}

/*
 *  Generate the Lights Out matrix.
 *
 *  The matrix of size (n^2)x(n^2) is made of n x n matrices of size n x n.
 *
 *  Matrices on the main diagonal are matrices with 1s on the main diagonal and neighbour diagonals. All other numbers are 0s.
 *  Matrices on neighbour diagonals are identity matrices.
 *  All othher matrices are zero matrices.
 *
 *  Example (3^2) x (3^2):
 *
 *  1 1 0   1 0 0   0 0 0
 *  1 1 1   0 1 0   0 0 0
 *  0 1 1   0 0 1   0 0 0
 *
 *  1 0 0   1 1 0   1 0 0
 *  0 1 0   1 1 1   0 1 0
 *  0 0 1   0 1 1   0 0 1
 *
 *  0 0 0   1 0 0   1 1 0
 *  0 0 0   0 1 0   1 1 1
 *  0 0 0   0 0 1   0 1 1
 *
 */
LOMatrix Solver::getSolvingMatrix(unsigned short size)
{
	int si_si = size*size;
	LOMatrix lightMatrix;

	for(int i = 0; i < si_si; i++)
	{
		lightMatrix.push_back(boost::dynamic_bitset<uint32_t>(si_si));
		lightMatrix[i].reset();
	}

	for(int i = 0; i < si_si; i++)
	{
		lightMatrix[i].set(i);

		if(i >= size)		 lightMatrix[i].set(i - size);
		if(i < si_si - size) lightMatrix[i].set(i + size);

		if(    i % size) lightMatrix[i].set(i - 1);
		if((i+1) % size) lightMatrix[i].set(i + 1);
	}

	return lightMatrix;
}

/* 
 * Invert the binary Lights Out matrix with gaussian inversion.
 *
 */
void Solver::getInverseMatrix(unsigned short size)
{
	int size_size = size*size;
	LOMatrix base = getSolvingMatrix(size);

	mInvSolvingMatrix.clear();

	for(int i = 0; i < size_size; i++)
	{
		mInvSolvingMatrix.push_back(boost::dynamic_bitset<uint32_t>(size_size));
		mInvSolvingMatrix[i].set(i);
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
					swap(mInvSolvingMatrix[i], mInvSolvingMatrix[j]);
					break;
				}
			}
		}

		for(int j = i + 1; j < size_size; j++)
		{
			if(base[j][i])
			{
				base[j] ^= base[i];
				mInvSolvingMatrix[j] ^= mInvSolvingMatrix[i];
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
				mInvSolvingMatrix[j] ^= mInvSolvingMatrix[i];
			}
		}
	}

	mSolvingInverted = true;
}

/*
* Solve the Lights Out puzzle.
*
*/
void Solver::SolveGame(LightsOutGame &game)
{
	//Exit if the game is already solved
	if(game.getField().none()) 
	{
		return;
	}

	//Change the current size of matrix if player has changed the size of game
	if(game.getSize() != mCurrentSize)
	{
		mSolvingInverted = false;
		mCurrentSize = game.getSize();
	}

	mTurns.clear();
	mSolution.clear();

	//Recompute solving matrix if player has changed the size of game
	if(!mSolvingInverted)
	{
		getInverseMatrix(game.getSize());
	}

	int si_si = game.getSize() * game.getSize();
	boost::dynamic_bitset<uint32_t> field = game.getField();
	mSolution.resize(si_si, false);

	//Calculate solution by multiplying inverse matrix on field vector
	for(int i = 0; i < si_si; i++)
	{
		mSolution[i] = (mInvSolvingMatrix[i] & field).count() & 0x01; //One if dot product of matrix row and field vector is odd, zero otherwise.

		if(mSolution[i])
		{
			mTurns.push_back(PointOnField((unsigned short)(i % game.getSize()), (unsigned short)(i / game.getSize())));
		}	
	}

	mSolvingFlag = true; //Now you can take turns from mTurns
}

/*
* Get the random turn from mTurns.
*
*/
PointOnField Solver::GetRandomTurn()
{	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> Distrib(0, mTurns.size() - 1);

	auto random_iterator = mTurns.begin() + Distrib(rd);

	PointOnField randomTurn = *random_iterator;
	mTurns.erase(random_iterator);

	if(mTurns.empty())
	{
		mSolvingFlag = false; //You cannot take turns from mTurns anymore
	}

	return randomTurn;
}

/*
* Get the first turn from mTurns.
*
*/
PointOnField Solver::GetFirstTurn()
{
	auto first_iterator = mTurns.begin();

	PointOnField firstTurn = *first_iterator;

	mTurns.erase(first_iterator);

	if(mTurns.empty())
	{
		mSolvingFlag = false; //You cannot take turns from mTurns anymore
	}

	return firstTurn;
}

/*
* Get the hint to solve the game.
*
*/
PointOnField Solver::GetHint(LightsOutGame &game)
{
	SolveGame(game);

	mSolvingFlag = false;

	if(!mTurns.empty())
	{
		return GetRandomTurn();
	}
	else
	{
		return PointOnField((unsigned short)(-1), (unsigned short)(-1));
	}
}

/*
* Get the whole solution.
*
*/
boost::dynamic_bitset<uint32_t> Solver::GetSolution(LightsOutGame &game)
{
	SolveGame(game);

	mSolvingFlag = false;

	return mSolution;
}

boost::dynamic_bitset<uint32_t> Solver::GetInverseSolution(LightsOutGame &game)
{
	mInverseSolution.clear();

	int si_si = game.getSize() * game.getSize();
	boost::dynamic_bitset<uint32_t> field = game.getField();
	mInverseSolution.resize(si_si, false);

	if(game.getField().empty())
	{
		return mInverseSolution;
	}

	for(int i = 0; i < field.size(); i++)
	{
		if(field[i])
		{
			mInverseSolution[i] ^= 1;

			uint32_t fieldX = i % game.getSize();
			uint32_t fieldY = i / game.getSize();

			int left   = fieldX >                  0 ? fieldX - 1 : -1;
			int right  = fieldX < game.getSize() - 1 ? fieldX + 1 : -1;
			int top    = fieldY >                  0 ? fieldY - 1 : -1;
			int bottom = fieldY < game.getSize() - 1 ? fieldY + 1 : -1;

			if(left != -1)
			{
				int sideIndex = fieldY * game.getSize() + left;
				mInverseSolution[sideIndex] ^= 1;
			}

			if (right != -1)
			{
				int sideIndex = fieldY * game.getSize() + right;
				mInverseSolution[sideIndex] ^= 1;
			}

			if (top != -1)
			{
				int sideIndex = top * game.getSize() + fieldX;
				mInverseSolution[sideIndex] ^= 1;
			}

			if (bottom != -1)
			{
				int sideIndex = bottom * game.getSize() + fieldX;
				mInverseSolution[sideIndex] ^= 1;
			}
		}
	}

	return mInverseSolution;
}

