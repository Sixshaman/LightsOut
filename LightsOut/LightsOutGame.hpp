#ifndef LIGHTS_OUT_GAME_HPP
#define LIGHTS_OUT_GAME_HPP

#include <boost\dynamic_bitset\dynamic_bitset.hpp>
#include <vector>
#include <Windows.h>

enum RESET_MODE
{
	RESET_FULL_RANDOM,     //Fully random field(Hotkey - F)
	RESET_SOLVABLE_RANDOM, //Solvable random field(Hotkey - R)
	RESET_ZERO_ELEMENT,	   //Fully turned off(Hotkey - 0)
	RESET_ONE_ELEMENT,	   //Fully turned on(Hotkey - 1)
	RESET_CLICK_ALL,	   //Field after all possible turns(Hotkey - A)
	RESET_BLATNOY,		   //Field with chessboard pattern(Hotkey - B)
	RESET_PETYA_STYLE,	   //Field with checkers pattern(Hotkey - P)
	RESET_BORDER,		   //Field fith only border enabled(Hotkey - O)
	RESET_RESOLVENT,	   //Field = given field(Hotkey - E)
	RESET_INVERSE          //Field = invert field
};

/*
* The Lights Out game class.
*
*/

class LightsOutGame
{
public:
	LightsOutGame(unsigned short size);
	~LightsOutGame();

	void Click(unsigned short xPos, unsigned short yPos);
	void ConstructClick(unsigned short xPos, unsigned short yPos);

	void ResetField(unsigned short size, RESET_MODE mode = RESET_SOLVABLE_RANDOM, boost::dynamic_bitset<UINT> *resolvent = nullptr);

	boost::dynamic_bitset<UINT> getField() const { return mMainField; }
	unsigned short getSize() const { return mSize; }

private:
	boost::dynamic_bitset<UINT> mMainField;
	unsigned short mSize;
};

#endif