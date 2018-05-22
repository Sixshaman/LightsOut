/*
Copyright (c) 2015 Sixshaman

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef LIGHTS_OUT_SOLVER_HPP
#define LIGHTS_OUT_SOLVER_HPP

/*
 * The solver class for Lights Out game.
 *
 */

#include "LightsOutGame.hpp"

struct PointOnField
{
	unsigned short field_X;
	unsigned short field_Y;

	PointOnField(unsigned short _x, unsigned short _y): field_X(_x), field_Y(_y) {}
	bool operator ==(PointOnField T) { return (field_X == T.field_X && field_Y == T.field_Y); };
};

typedef std::vector<boost::dynamic_bitset<UINT>> LOMatrix;

class Solver
{
public:
	Solver();
	bool GetsolvingFlag() const { return mSolvingFlag; }

	void SolveGame(LightsOutGame &game);
	PointOnField GetHint(LightsOutGame &game);

	boost::dynamic_bitset<UINT> GetResolvent(LightsOutGame &game);
	boost::dynamic_bitset<UINT> GetInverseResolvent(LightsOutGame &game);

	/*[deprecated]*/
	//void SortStrokesByCoolness(LightsOutGame &game); //DO NOT CALL THIS METHOD

	PointOnField GetRandomTurn();
	PointOnField GetFirstTurn();

private:
	LOMatrix getSolvingMatrix(unsigned short size);
	void getInverseMatrix(unsigned short size);

	/*[deprecated]*/
	//int getNearestLen(LightsOutGame &game, PointOnField p);  //DO NOT CALL THIS METHOD
		
private:
	bool mSolvingFlag; //True if you are allowed to take values from mTurns

	boost::dynamic_bitset<UINT> mResolvent; //The solution in dynamic_bitset representation
	std::vector<PointOnField> mTurns;		//The solution in PointOnField representation

	boost::dynamic_bitset<UINT> mInverseResolvent; //The anti-solution in dynamic_bitset representation

	LOMatrix mInvSolvingMatrix;  //
	bool mSolvingInversed;		 //For solution caching 
	unsigned short mCurrentSize; //
};

#endif