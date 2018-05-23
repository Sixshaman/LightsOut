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

#include "LightsOutGame.hpp"
#include "Util.hpp"
#include <random>

LightsOutGame::LightsOutGame(unsigned short size): mSize(size)
{
	ResetField(mSize);
}

LightsOutGame::~LightsOutGame()
{
}

void LightsOutGame::ResetField(unsigned short size, RESET_MODE mode, boost::dynamic_bitset<UINT> *resolvent)
{
	mSize = size;
	UINT si_si = size*size;

	mMainField.clear();
	mMainField.resize(si_si);
	
	mMainField.reset();

	std::random_device rd;

	for(unsigned short i = 0; i < si_si; i++)
	{
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> booleanDistrib(0, 1);

		switch(mode)
		{
		case RESET_FULL_RANDOM:
			mMainField.set(i, booleanDistrib(gen) == 1);
			break;
		case RESET_SOLVABLE_RANDOM:
			if(booleanDistrib(gen) == 1)
			{
				Click(i / size, i % size);
			}
			break;
		case RESET_ZERO_ELEMENT:
			mMainField.set(i, false);
			break;
		case RESET_ONE_ELEMENT:
			mMainField.set(i, true);
			break;
		case RESET_CLICK_ALL:
			Click(i / size, i % size);
			break;
		case RESET_BLATNOY:
			if((i / size)%2 == (i % size)%2)
				mMainField.set(i, 1);
			else
				mMainField.set(i, 0);
			break;
		case RESET_PETYA_STYLE:
			if((i / size) % 2)
			{
				if(!((i % size) % 2))
					mMainField.set(i, 1);
				else
					mMainField.set(i, 0);
			}
			else
			{
				mMainField.set(i, 0);
			}
			break;
		case RESET_BORDER:
			if(i/size == 0 || i/size == (size - 1) || i%size == 0 || i%size == (size - 1))
			{
				mMainField.set(i, 1);
			}
			else
			{
				mMainField.set(i, 0);
			}
			break;
		case RESET_RESOLVENT:
			if(resolvent)
			{
				mMainField = *resolvent;
			}
			break;
		}
	}
}

void LightsOutGame::Click(unsigned short posX, unsigned short posY)
{
	Clamp(posX, (unsigned short)0, (unsigned short)(mSize - 1));
	Clamp(posY, (unsigned short)0, (unsigned short)(mSize - 1));

	mMainField[posY*mSize + posX].flip();
	if(posX > 0) mMainField[posY*mSize + posX - 1].flip();
	if(posX < mSize - 1) mMainField[posY*mSize + posX + 1].flip();
	if(posY > 0) mMainField[(posY-1)*mSize + posX].flip();
	if(posY < mSize - 1) mMainField[(posY+1)*mSize + posX].flip();
}

void LightsOutGame::ConstructClick(unsigned short posX, unsigned short posY)
{
	Clamp(posX, (unsigned short)0, (unsigned short)(mSize - 1));
	Clamp(posY, (unsigned short)0, (unsigned short)(mSize - 1));

	mMainField[posY*mSize + posX].flip();
}