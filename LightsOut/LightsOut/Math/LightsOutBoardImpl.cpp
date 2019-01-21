#include "LightsOutBoardImpl.hpp"

LightsOutBoardImpl::LightsOutBoardImpl(uint16_t gameSize): mSize(gameSize)
{
}

LightsOutBoardImpl::~LightsOutBoardImpl()
{
}

uint32_t LightsOutBoardImpl::CellCount() const
{
	return mSize * mSize;
}

uint16_t LightsOutBoardImpl::Size() const
{
	return mSize;
}
