#include "LightsOutBoard.hpp"
#include "LightsOutBoardImpl.hpp"

LightsOutBoard::LightsOutBoard(uint16_t gameSize, uint16_t domainSize): mpImpl(nullptr)
{
	if(domainSize == 2)
	{
		mpImpl = new LightsOutBinaryBoardImpl(gameSize);
	}
	else
	{
		mpImpl = new LightsOutNaryBoardImpl(gameSize, domainSize);
	}
}

LightsOutBoard::~LightsOutBoard()
{
	delete mpImpl;
}

LightsOutBoard::LightsOutBoard(const LightsOutBoard& right)
{
	if(right.DomainSize() == 2)
	{
		mpImpl = new LightsOutBinaryBoardImpl(*(reinterpret_cast<LightsOutBinaryBoardImpl*>(right.mpImpl)));
	}
	else
	{
		mpImpl = new LightsOutNaryBoardImpl(*(reinterpret_cast<LightsOutNaryBoardImpl*>(right.mpImpl)));
	}
}

LightsOutBoard LightsOutBoard::operator=(const LightsOutBoard & right)
{
	if (right.DomainSize() == 2)
	{
		mpImpl = new LightsOutBinaryBoardImpl(*(reinterpret_cast<LightsOutBinaryBoardImpl*>(right.mpImpl)));
	}
	else
	{
		mpImpl = new LightsOutNaryBoardImpl(*(reinterpret_cast<LightsOutNaryBoardImpl*>(right.mpImpl)));
	}

	return *this;
}

bool LightsOutBoard::operator==(const LightsOutBoard & right)
{
	if(DomainSize() != right.DomainSize() || Size() != right.Size())
	{
		return false;
	}
	else
	{
		if (DomainSize() == 2)
		{
			LightsOutBinaryBoardImpl* binLeft  = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
			LightsOutBinaryBoardImpl* binRight = reinterpret_cast<LightsOutBinaryBoardImpl*>(right.mpImpl);

			return binLeft->IsEqual(binRight);
		}
		else
		{
			LightsOutNaryBoardImpl* nLeft  = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
			LightsOutNaryBoardImpl* nRight = reinterpret_cast<LightsOutNaryBoardImpl*>(right.mpImpl);

			return nLeft->IsEqual(nRight);
		}
	}
}

uint32_t LightsOutBoard::CellCount() const
{
	return mpImpl->CellCount();
}

uint16_t LightsOutBoard::Size() const
{
	return mpImpl->Size();
}

uint16_t LightsOutBoard::DomainSize() const
{
	return mpImpl->DomainSize();
}

void LightsOutBoard::Reset(uint16_t gameSize)
{
	mpImpl->Reset(gameSize);
}

void LightsOutBoard::SetCellValue(uint16_t x, uint16_t y, uint16_t value)
{
	mpImpl->Set(x, y, value);
}

uint16_t LightsOutBoard::GetCellValue(uint16_t x, uint16_t y) const
{
	return mpImpl->Get(x, y);
}

void LightsOutBoard::IncrementCell(uint16_t x, uint16_t y)
{
	mpImpl->IncrementCell(x, y);
}

void LightsOutBoard::BoardAdd(const LightsOutBoard& board)
{
	if(DomainSize() != board.DomainSize() || Size() != board.Size())
	{
		return;
	}

	if(DomainSize() == 2)
	{
		LightsOutBinaryBoardImpl* binLeft  = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
		LightsOutBinaryBoardImpl* binRight = reinterpret_cast<LightsOutBinaryBoardImpl*>(board.mpImpl);

		binLeft->AddBoard(binRight);
	}
	else
	{
		LightsOutNaryBoardImpl* nLeft  = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		LightsOutNaryBoardImpl* nRight = reinterpret_cast<LightsOutNaryBoardImpl*>(board.mpImpl);

		nLeft->AddBoard(nRight);
	}
}

void LightsOutBoard::BoardSub(const LightsOutBoard& board)
{
	if (DomainSize() != board.DomainSize() || Size() != board.Size())
	{
		return;
	}

	if (DomainSize() == 2)
	{
		LightsOutBinaryBoardImpl* binLeft = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
		LightsOutBinaryBoardImpl* binRight = reinterpret_cast<LightsOutBinaryBoardImpl*>(board.mpImpl);

		binLeft->AddBoard(binRight);
	}
	else
	{
		LightsOutNaryBoardImpl* nLeft = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		LightsOutNaryBoardImpl* nRight = reinterpret_cast<LightsOutNaryBoardImpl*>(board.mpImpl);

		nLeft->SubBoard(nRight);
	}
}

void LightsOutBoard::BoardMul(const LightsOutBoard& board)
{
	if (DomainSize() != board.DomainSize() || Size() != board.Size())
	{
		return;
	}

	if(DomainSize() == 2)
	{
		LightsOutBinaryBoardImpl* binLeft  = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
		LightsOutBinaryBoardImpl* binRight = reinterpret_cast<LightsOutBinaryBoardImpl*>(board.mpImpl);

		binLeft->MulBoard(binRight);
	}
	else
	{
		LightsOutNaryBoardImpl* nLeft  = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		LightsOutNaryBoardImpl* nRight = reinterpret_cast<LightsOutNaryBoardImpl*>(board.mpImpl);

		nLeft->MulBoard(nRight);
	}
}

void LightsOutBoard::BoardMulComponentWise(const LightsOutBoard& board)
{
	if (DomainSize() != board.DomainSize() || Size() != board.Size())
	{
		return;
	}

	if(DomainSize() == 2)
	{
		LightsOutBinaryBoardImpl* binLeft = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
		LightsOutBinaryBoardImpl* binRight = reinterpret_cast<LightsOutBinaryBoardImpl*>(board.mpImpl);

		binLeft->MulBoardComponentWise(binRight);
	}
	else
	{
		LightsOutNaryBoardImpl* nLeft = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		LightsOutNaryBoardImpl* nRight = reinterpret_cast<LightsOutNaryBoardImpl*>(board.mpImpl);

		nLeft->MulBoardComponentWise(nRight);
	}
}

void LightsOutBoard::BoardMulNum(uint16_t mul)
{
	if(DomainSize() == 2)
	{
		if(mul == 0)
		{
			mpImpl->Reset(Size());
		}
	}
	else
	{
		LightsOutNaryBoardImpl* boardNaryImpl = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		boardNaryImpl->MulBoardNum(mul);
	}
}

void LightsOutBoard::BoardSubMul(const LightsOutBoard& board, uint16_t mul)
{
	if(DomainSize() != board.DomainSize() || Size() != board.Size() || mul == 0)
	{
		return;
	}

	if(DomainSize() == 2)
	{
		LightsOutBinaryBoardImpl* binLeft = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
		LightsOutBinaryBoardImpl* binRight = reinterpret_cast<LightsOutBinaryBoardImpl*>(board.mpImpl);

		binLeft->AddBoard(binRight);
	}
	else
	{
		LightsOutNaryBoardImpl* nLeft = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		LightsOutNaryBoardImpl* nRight = reinterpret_cast<LightsOutNaryBoardImpl*>(board.mpImpl);

		nLeft->SubMulBoard(nRight, mul);
	}
}

void LightsOutBoard::BoardIncDif(const LightsOutBoard& board1, const LightsOutBoard& board2)
{
	if(DomainSize() != board1.DomainSize() || DomainSize() != board2.DomainSize() || Size() != board1.Size() || Size() != board2.Size())
	{
		return;
	}

	if (DomainSize() == 2)
	{
		LightsOutBinaryBoardImpl* binLeft   = reinterpret_cast<LightsOutBinaryBoardImpl*>(mpImpl);
		LightsOutBinaryBoardImpl* binRight1 = reinterpret_cast<LightsOutBinaryBoardImpl*>(board1.mpImpl);
		LightsOutBinaryBoardImpl* binRight2 = reinterpret_cast<LightsOutBinaryBoardImpl*>(board2.mpImpl);

		binLeft->IncDifBoard(binRight1, binRight2);
	}
	else
	{
		LightsOutNaryBoardImpl* nLeft   = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		LightsOutNaryBoardImpl* nRight1 = reinterpret_cast<LightsOutNaryBoardImpl*>(board1.mpImpl);
		LightsOutNaryBoardImpl* nRight2 = reinterpret_cast<LightsOutNaryBoardImpl*>(board2.mpImpl);

		nLeft->IncDifBoard(nRight1, nRight2);
	}
}

void LightsOutBoard::DomainRotate()
{
	mpImpl->Flip();
}

void LightsOutBoard::DomainRotateNonZero()
{
	if(DomainSize() > 2)
	{
		LightsOutNaryBoardImpl* boardNaryImpl = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		boardNaryImpl->RotateNonZero();
	}
}

void LightsOutBoard::InvertValues()
{
	if (DomainSize() > 2)
	{
		LightsOutNaryBoardImpl* boardNaryImpl = reinterpret_cast<LightsOutNaryBoardImpl*>(mpImpl);
		boardNaryImpl->InvertValues();
	}
}

bool LightsOutBoard::IsNone() const
{
	return mpImpl->IsNone();
}

void LightsOutBoard::CopyMemoryData(uint32_t* buf, uint32_t maxSize) const
{
	mpImpl->CopyMemoryData(buf, maxSize);
}
