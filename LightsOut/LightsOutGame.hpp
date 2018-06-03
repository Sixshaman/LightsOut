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
	RESET_BLATNOY,		   //Field with chessboard pattern(Hotkey - B)
	RESET_PETYA_STYLE,	   //Field with checkers pattern(Hotkey - P)
	RESET_BORDER,		   //Field fith only border enabled(Hotkey - O)
	RESET_RESOLVENT,	   //Field = given field(Hotkey - E)
	RESET_INVERSE          //Field = inverted field
};

/*
* The Lights Out game class.
*
*/

class LightsOutGame
{
public:
	LightsOutGame();
	~LightsOutGame();

	void Click(unsigned short xPos, unsigned short yPos);
	void ConstructClick(unsigned short xPos, unsigned short yPos);

	void ResetField(unsigned short size, RESET_MODE mode = RESET_SOLVABLE_RANDOM, boost::dynamic_bitset<uint32_t> *resolvent = nullptr);
	void ResetStability();

	boost::dynamic_bitset<uint32_t> getField()     const { return mMainField; }
	boost::dynamic_bitset<uint32_t> getStability() const { return mStability; }
	unsigned short                  getSize()      const { return mSize; }

private:
	boost::dynamic_bitset<uint32_t> mMainField;
	boost::dynamic_bitset<uint32_t> mStability;
	unsigned short mSize;
};

#endif