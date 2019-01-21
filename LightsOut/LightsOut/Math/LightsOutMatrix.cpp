#include "LightsOutMatrix.hpp"
#include "..\Util.hpp"
#include <string>

LightsOutMatrix::LightsOutMatrix(uint16_t gameSize, uint16_t domainSize): mDomainSize(domainSize)
{
	Resize(gameSize);
}

LightsOutMatrix::~LightsOutMatrix()
{
}

uint32_t LightsOutMatrix::GetGameSize()
{
	return mSizeSmall;
}

uint32_t LightsOutMatrix::GetMatrixSize()
{
	return mSizeBig;
}

uint16_t LightsOutMatrix::GetDomainSize()
{
	return mDomainSize;
}

void LightsOutMatrix::Resize(uint16_t gameSize)
{
	mSizeSmall = gameSize;
	mSizeBig   = mSizeSmall * mSizeSmall;

	LightsOutBoard board(gameSize, mDomainSize);
	mCellClickRules.resize(mSizeBig, board);
}

void LightsOutMatrix::SetVal(uint16_t cellRuleX, uint16_t cellRuleY, uint16_t x, uint16_t y, uint16_t val)
{
	uint32_t row = cellRuleY * mSizeSmall + cellRuleX;
	mCellClickRules[row].SetCellValue(x, y, val);
}

uint16_t LightsOutMatrix::GetVal(uint16_t cellRuleX, uint16_t cellRuleY, uint16_t x, uint16_t y)
{
	uint32_t row = cellRuleY * mSizeSmall + cellRuleX;
	return mCellClickRules[row].GetCellValue(x, y);
}

LightsOutBoard LightsOutMatrix::GetCellClickRule(uint16_t cellRuleX, uint16_t cellRuleY)
{
	uint32_t row = cellRuleY * mSizeSmall + cellRuleX;
	return mCellClickRules[row];
}

void LightsOutMatrix::SetCellClickRule(uint16_t cellRuleX, uint16_t cellRuleY, const LightsOutBoard& board)
{
	uint32_t row = cellRuleY * mSizeSmall + cellRuleX;
	mCellClickRules[row] = board;
}

uint32_t LightsOutMatrix::Inverto()
{
	std::vector<LightsOutBoard> invMatrix;
	invMatrix.reserve(mSizeBig);
	for(uint16_t y = 0; y < mSizeSmall; y++)
	{
		for(uint16_t x = 0; x < mSizeSmall; x++)
		{
			LightsOutBoard emptyBoard(mSizeSmall, mDomainSize);
			emptyBoard.SetCellValue(x, y, 1);
			invMatrix.push_back(emptyBoard);
		}
	}

	std::vector<uint16_t> domainInvs;
	domainInvs.resize(mDomainSize);
	for(size_t i = 0; i < mDomainSize; i++)
	{
		domainInvs[i] = (uint16_t)InvModGcdex(i, mDomainSize);
	}

	for(uint32_t i = 0; i < mSizeBig; i++)
	{
		uint16_t xMinor = i % mSizeSmall;
		uint16_t yMinor = i / mSizeSmall;

		uint16_t xMajor = xMinor;
		uint16_t yMajor = yMinor;

		uint16_t thisVal = GetVal(xMajor, yMajor, xMinor, yMinor);
		uint16_t compVal = GetVal(xMajor, yMajor, xMinor, yMinor);
		if(domainInvs[compVal] == 0 || (thisVal != 1 && mDomainSize % thisVal == 0))
		{
			for(uint32_t j = i + 1; j < mSizeBig; j++)
			{
				xMajor  = j % mSizeSmall;
				yMajor  = j / mSizeSmall;
				compVal = GetVal(xMajor, yMajor, xMinor, yMinor);

				if(domainInvs[compVal] != 0)
				{
					thisVal = compVal;
					std::swap(mCellClickRules[i], mCellClickRules[j]);
					std::swap(invMatrix[i],       invMatrix[j]);
					break;
				}
			}
		}

		for(int j = i + 1; j < mSizeBig; j++)
		{
			xMajor  = j % mSizeSmall;
			yMajor  = j / mSizeSmall;
			compVal = GetVal(xMajor, yMajor, xMinor, yMinor);
			
			if(domainInvs[compVal] != 0)
			{
				int32_t invThisVal = domainInvs[thisVal];
				mCellClickRules[j].BoardSubMul(mCellClickRules[i], invThisVal * compVal);
				invMatrix[j].BoardSubMul(invMatrix[i], invThisVal * compVal);
			}
		}
	}

	uint32_t qpCount = 0;
	for(int32_t i = mSizeBig - 1; i >= 0; i--)
	{
		uint16_t xMinor = i % mSizeSmall;
		uint16_t yMinor = i / mSizeSmall;

		uint16_t thisVal = GetVal(xMinor, yMinor, xMinor, yMinor);
		for (int j = i - 1; j >= 0; j--)
		{
			uint16_t xMajor  = j % mSizeSmall;
			uint16_t yMajor  = j / mSizeSmall;
			uint16_t compVal = GetVal(xMajor, yMajor, xMinor, yMinor);

			if(domainInvs[compVal] != 0)
			{
				int32_t invThisVal = domainInvs[thisVal];
				mCellClickRules[j].BoardSubMul(mCellClickRules[i], invThisVal * compVal);
				invMatrix[j].BoardSubMul(invMatrix[i], invThisVal * compVal);
			}
		}

		if(domainInvs[thisVal] != 0)
		{
			uint16_t invThisVal = domainInvs[thisVal];
			mCellClickRules[i].BoardMulNum(invThisVal);
			invMatrix[i].BoardMulNum(invThisVal);
		}

		if(mCellClickRules[i].IsNone())
		{
			qpCount++;
		}
	}

	for (int i = 0; i < mSizeBig; i++)
	{
		uint16_t xMajor = i % mSizeSmall;
		uint16_t yMajor = i / mSizeSmall;

		for (int j = 0; j < i; j++)
		{
			uint16_t xMinor = j % mSizeSmall;
			uint16_t yMinor = j / mSizeSmall;

			uint16_t temp = invMatrix[i].GetCellValue(xMinor, yMinor);
			invMatrix[i].SetCellValue(xMinor, yMinor, invMatrix[j].GetCellValue(xMajor, yMajor));
			invMatrix[j].SetCellValue(xMajor, yMajor, temp);
		}
	}

	mCellClickRules = invMatrix;

	return qpCount;
}
