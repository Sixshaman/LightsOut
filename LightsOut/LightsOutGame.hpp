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