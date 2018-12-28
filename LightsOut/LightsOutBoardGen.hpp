#pragma once

#ifndef LIGHTS_OUT_GAME_HPP
#define LIGHTS_OUT_GAME_HPP

#include <boost\dynamic_bitset\dynamic_bitset.hpp>
#include <vector>
#include <Windows.h>

enum class BoardGenMode
{
	BOARDGEN_FULL_RANDOM,  //Fully random board(Hotkey - F)
	BOARDGEN_ZERO_ELEMENT, //Fully turned off(Hotkey - 0)
	BOARDGEN_ONE_ELEMENT,  //Fully turned on(Hotkey - 1)
	BOARDGEN_BLATNOY,	   //Board with chessboard pattern(Hotkey - B)
	BOARDGEN_PETYA_STYLE,  //Board with checkers pattern(Hotkey - P)
	BOARDGEN_BORDER,	   //Board fith only border enabled(Hotkey - O)
};

/*
* The Lights Out game class.
*
*/

class LightsOutBoardGen
{
public:
	LightsOutBoardGen();
	~LightsOutBoardGen();

	boost::dynamic_bitset<uint32_t> Generate(unsigned short size, BoardGenMode mode = BoardGenMode::BOARDGEN_FULL_RANDOM);
};

#endif