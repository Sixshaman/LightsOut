#include "LightsOutBoardImpl.hpp"

LightsOutBinaryBoardImpl::LightsOutBinaryBoardImpl(uint16_t gameSize): LightsOutBoardImpl(gameSize)
{
	Reset(gameSize);
}

LightsOutBinaryBoardImpl::~LightsOutBinaryBoardImpl()
{
}

uint16_t LightsOutBinaryBoardImpl::DomainSize() const
{
	return 2;
}

void LightsOutBinaryBoardImpl::Reset(uint16_t gameSize)
{
	mSize = gameSize;
	mBoard.clear();
	mBoard.resize(gameSize * gameSize, false);
}

void LightsOutBinaryBoardImpl::Set(uint16_t x, uint16_t y, uint16_t value)
{
	uint32_t index = y * mSize + x;
	mBoard.set(index, value > 0);
}

uint16_t LightsOutBinaryBoardImpl::Get(uint16_t x, uint16_t y) const
{
	return mBoard[y * mSize + x];
}

void LightsOutBinaryBoardImpl::IncrementCell(uint16_t x, uint16_t y)
{
	mBoard[y * mSize + x].flip();
}

void LightsOutBinaryBoardImpl::AddBoard(const LightsOutBinaryBoardImpl* right)
{
	mBoard = mBoard ^ right->mBoard;
}

void LightsOutBinaryBoardImpl::MulBoard(const LightsOutBinaryBoardImpl* right)
{
	bool mulBit = (mBoard & right->mBoard).count() & 0x01;
	mBoard.set(0, mulBit);
}

void LightsOutBinaryBoardImpl::MulBoardComponentWise(const LightsOutBinaryBoardImpl* right)
{
	mBoard = mBoard & right->mBoard;
}

void LightsOutBinaryBoardImpl::IncDifBoard(const LightsOutBinaryBoardImpl* right1, const LightsOutBinaryBoardImpl* right2)
{
	mBoard &= (~(right1->mBoard ^ right2->mBoard));
}

bool LightsOutBinaryBoardImpl::IsEqual(const LightsOutBinaryBoardImpl* right)
{
	return mBoard == right->mBoard;
}

void LightsOutBinaryBoardImpl::Flip()
{
	mBoard.flip();
}

bool LightsOutBinaryBoardImpl::IsNone() const
{
	return mBoard.none();
}

void LightsOutBinaryBoardImpl::CopyMemoryData(uint32_t* buf, uint32_t maxSize) const
{
	if(maxSize >= mBoard.size())
	{
		boost::to_block_range<uint32_t>(mBoard, buf);
	}
}
