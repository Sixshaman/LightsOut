#include "Util.hpp"
#include <comdef.h>

int32_t InvModGcdex(int32_t x, int32_t domainSize)
{
	if (x == 1)
	{
		return 1;
	}
	else
	{
		if (x == 0 || domainSize % x == 0)
		{
			return 0;
		}
		else
		{
			int32_t tCurr = 0;
			int32_t rCurr = domainSize;
			int32_t tNext = 1;
			int32_t rNext = x;

			while (rNext != 0)
			{
				int32_t quotR = rCurr / rNext;
				int32_t tPrev = tCurr;
				int32_t rPrev = rCurr;

				tCurr = tNext;
				rCurr = rNext;

				tNext = tPrev - quotR * tCurr;
				rNext = rPrev - quotR * rCurr;
			}

			tCurr = (tCurr + domainSize) % domainSize;
			return tCurr;
		}
	}
}

DXException::DXException(HRESULT hr, const std::wstring& funcName, const std::wstring& filename, int32_t line)
{
	mErrorCode  = hr;
	mFuncName   = funcName;
	mFilename   = filename;
	mLineNumber = line;
}

std::wstring DXException::ToString() const
{
	_com_error err(mErrorCode);
	std::wstring msg = err.ErrorMessage();

	return mFuncName + L" failed in " + mFilename + L"; line " + std::to_wstring(mLineNumber) + L"; error: " + msg;
}
