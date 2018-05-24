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