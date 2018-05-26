#include "LightsOutSolver.hpp"
#include <algorithm>
#include <random>
#include "Util.hpp"

Solver::Solver(): mSolvingFlag(false), mResolvent(boost::dynamic_bitset<uint32_t>()), mSolvingInversed(false), mCurrentSize(0)
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

	mSolvingInversed = true;
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
		mSolvingInversed = false;
		mCurrentSize = game.getSize();
	}

	mTurns.clear();
	mResolvent.clear();

	//Recompute solving matrix if player has changed the size of game
	if(!mSolvingInversed)
	{
		getInverseMatrix(game.getSize());
	}

	int si_si = game.getSize() * game.getSize();
	boost::dynamic_bitset<uint32_t> field = game.getField();
	mResolvent.resize(si_si, false);

	//Calculate solution by multiplying inverse matrix on field vector
	for(int i = 0; i < si_si; i++)
	{
		mResolvent[i] = (mInvSolvingMatrix[i] & field).count() & 0x01; //One if dot product of matrix row and field vector is odd, zero otherwise.

		if(mResolvent[i])
		{
			mTurns.push_back(PointOnField((unsigned short)(i % game.getSize()), (unsigned short)(i / game.getSize())));
		}
	}

	mSolvingFlag = true; //Now you can take turns from mTurns
}

/*
 * -----DEPRECATED-----
 *
 * This method must return manhattan distance to the nearest "on" cell for some cell p.
 *
 * However, it is extremely slow and doesn't work properly.
 *
 * Do not uncomment it, please.
 *
 */
//int Solver::getNearestLen(LightsOutGame &game, PointOnField p)
//{
//	int len = 0;
//
//	for(int i = 0; len == 0; i++)
//	{
//		if(i >= game.getSize() / 2 + 1)
//		{
//			return -1;
//		}
//
//		for(int j = -i; j <= i && len != 0; j++)
//		{
//			int xNew = p.field_X + j;
//			int yNew = p.field_Y - i;
//
//			Clamp(xNew, 0, (int)game.getSize());
//			Clamp(yNew, 0, (int)game.getSize());
//
//			if(game.getField()[yNew*game.getSize() + xNew])
//			{
//				len = i;
//			}
//		}
//		for(int j = -i + 1; j <= i - 1; j++)
//		{
//			int yNew = p.field_Y + j;
//			int xNewLeft = p.field_X - i;
//			int xNewRight = p.field_X + i;
//
//			Clamp(yNew, 0, (int)game.getSize());
//			Clamp(xNewLeft, 0, (int)game.getSize());
//			Clamp(xNewRight, 0, (int)game.getSize());
//
//			if(game.getField()[yNew*game.getSize() + xNewLeft] || game.getField()[yNew*game.getSize() + xNewRight])
//			{
//				len = i;
//			}
//		}
//		for(int j = -i; j <= i; j++)
//		{
//			int xNew = p.field_X + j;
//			int yNew = p.field_Y - i;
//
//			Clamp(xNew, 0, (int)game.getSize());
//			Clamp(yNew, 0, (int)game.getSize());
//
//			if(game.getField()[yNew*game.getSize() + xNew])
//			{
//				len = i;
//			}
//		}
//	}
//
//	return len;
//}

/*
* -----DEPRECATED-----
*
* This method must sort all turns in order of closeness to nearest "on" cells.
*
* However, it is extremely slow in some cases and doesn't work properly.
*
* Do not uncomment it, please.
*
*/
//void Solver::sortTurnsByCoolness(LightsOutGame &game)
//{
//	std::sort(mTurns.begin(), mTurns.end(), [&game, this](PointOnField a, PointOnField b)
//	{
//		return getNearestLen(game, a) < getNearestLen(game, b);
//	});
//}


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

	return mResolvent;
}

boost::dynamic_bitset<uint32_t> Solver::GetInverseResolvent(LightsOutGame &game)
{
	mInverseResolvent.clear();

	int si_si = game.getSize() * game.getSize();
	boost::dynamic_bitset<uint32_t> field = game.getField();
	mInverseResolvent.resize(si_si, false);

	if(game.getField().empty())
	{
		return mInverseResolvent;
	}

	for(int i = 0; i < field.size(); i++)
	{
		if(field[i])
		{
			mInverseResolvent[i] ^= 1;

			uint32_t fieldX = i % game.getSize();
			uint32_t fieldY = i / game.getSize();

			int left   = fieldX >                  0 ? fieldX - 1 : -1;
			int right  = fieldX < game.getSize() - 1 ? fieldX + 1 : -1;
			int top    = fieldY >                  0 ? fieldY - 1 : -1;
			int bottom = fieldY < game.getSize() - 1 ? fieldY + 1 : -1;

			if(left != -1)
			{
				int sideIndex = fieldY * game.getSize() + left;
				mInverseResolvent[sideIndex] ^= 1;
			}

			if (right != -1)
			{
				int sideIndex = fieldY * game.getSize() + right;
				mInverseResolvent[sideIndex] ^= 1;
			}

			if (top != -1)
			{
				int sideIndex = top * game.getSize() + fieldX;
				mInverseResolvent[sideIndex] ^= 1;
			}

			if (bottom != -1)
			{
				int sideIndex = bottom * game.getSize() + fieldX;
				mInverseResolvent[sideIndex] ^= 1;
			}
		}
	}

	return mInverseResolvent;
}

