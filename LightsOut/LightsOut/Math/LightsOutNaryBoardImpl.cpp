#include "LightsOutBoardImpl.hpp"

LightsOutNaryBoardImpl::LightsOutNaryBoardImpl(uint16_t gameSize, uint16_t domainSize): LightsOutBoardImpl(gameSize), mDomainSize(domainSize)
{
	Reset(gameSize);
}

LightsOutNaryBoardImpl::~LightsOutNaryBoardImpl()
{
}

uint16_t LightsOutNaryBoardImpl::DomainSize() const
{
	return mDomainSize;
}

void LightsOutNaryBoardImpl::Reset(uint16_t gameSize)
{
	mSize = gameSize;
	mBoard.resize(gameSize * gameSize, 0);
}

void LightsOutNaryBoardImpl::Set(uint16_t x, uint16_t y, uint16_t value)
{
	uint32_t index   = y * mSize + x;
	mBoard.at(index) = value;
}

uint16_t LightsOutNaryBoardImpl::Get(uint16_t x, uint16_t y) const
{
	uint32_t index = y * mSize + x;
	return mBoard.at(index);
}

void LightsOutNaryBoardImpl::IncrementCell(uint16_t x, uint16_t y)
{
	uint32_t index   = y * mSize + x;
	mBoard.at(index) = (mBoard.at(index) + 1) % mDomainSize;
}

void LightsOutNaryBoardImpl::AddBoard(const LightsOutNaryBoardImpl* right)
{
	for(uint32_t i = 0; i < mBoard.size(); i++) //TODO: SSE
	{
		mBoard[i] = ((int32_t)mBoard[i] + (int32_t)right->mBoard[i]) % mDomainSize;
	}
}

void LightsOutNaryBoardImpl::SubBoard(const LightsOutNaryBoardImpl* right)
{
	for (uint32_t i = 0; i < mBoard.size(); i++) //TODO: SSE
	{
		mBoard[i] = ((((int32_t)mBoard[i] - (int32_t)right->mBoard[i]) % mDomainSize) + mDomainSize) % mDomainSize;
	}
}

void LightsOutNaryBoardImpl::MulBoard(const LightsOutNaryBoardImpl* right)
{
	uint64_t sum = 0;
	for(uint32_t i = 0; i < mBoard.size(); i++)
	{
		sum += ((uint32_t)mBoard[i] * (int32_t)right->mBoard[i]);
	}

	sum = sum % mDomainSize;
	mBoard[0] = (uint16_t)sum;
}

void LightsOutNaryBoardImpl::MulBoardComponentWise(const LightsOutNaryBoardImpl * right)
{
	for(uint32_t i = 0; i < mBoard.size(); i++) //TODO: SSE
	{
		mBoard[i] = ((((int32_t)mBoard[i] * (int32_t)right->mBoard[i]) % mDomainSize) + mDomainSize) % mDomainSize;
	}
}

void LightsOutNaryBoardImpl::MulBoardNum(uint16_t mul)
{
	for(uint32_t i = 0; i < mBoard.size(); i++) //TODO: SSE
	{
		mBoard[i] = (mBoard[i] * mul) % mDomainSize;
	}
}

void LightsOutNaryBoardImpl::SubMulBoard(const LightsOutNaryBoardImpl* right, uint16_t mul)
{
	if(mul == 0)
	{
		return;
	}

	for(uint32_t i = 0; i < mBoard.size(); i++) //TODO: SSE
	{
		int32_t res = (int32_t)mBoard[i] - (int32_t)mul * (int32_t)right->mBoard[i];
		mBoard[i] = ((res % mDomainSize) + mDomainSize) % mDomainSize;
	}
}

void LightsOutNaryBoardImpl::IncDifBoard(const LightsOutNaryBoardImpl* right1, const LightsOutNaryBoardImpl* right2)
{
	//Possibly impossible to implement. We don't know the theory
}

bool LightsOutNaryBoardImpl::IsEqual(const LightsOutNaryBoardImpl* right)
{
	for(uint32_t i = 0; i < mBoard.size(); i++)
	{
		if(mBoard[i] != right->mBoard[i])
		{
			return false;
		}
	}

	return true;
}

void LightsOutNaryBoardImpl::Flip()
{
	for(uint32_t& cell: mBoard)
	{
		cell = (cell + 1) % mDomainSize;
	}
}

void LightsOutNaryBoardImpl::RotateNonZero()
{
	for(uint32_t& cell : mBoard)
	{
		if(cell > 0)
		{
			cell = (cell) % (mDomainSize - 1) + 1;
		}
	}
}

void LightsOutNaryBoardImpl::InvertValues()
{
	for(uint32_t& cell: mBoard)
	{
		cell = (mDomainSize - cell) % mDomainSize;
	}
}

bool LightsOutNaryBoardImpl::IsNone() const
{
	bool nonZero = false;
	for(uint32_t cell: mBoard)
	{
		if(cell != 0 && (cell == 1 || mDomainSize % cell != 0))
		{
			nonZero = true;
			break;
		}
	}

	return !nonZero;
}

void LightsOutNaryBoardImpl::CopyMemoryData(uint32_t* buf, uint32_t maxSize) const
{
	if(maxSize >= mBoard.size())
	{
		memcpy_s(buf, maxSize * sizeof(uint32_t), mBoard.data(), mBoard.size() * sizeof(uint32_t));
	}
}
