#ifndef UTIL_HPP
#define UTIL_HPP

#include <tchar.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>

/*
* Some utilites
*
*/

#define SafeRelease(x) {if(x) x->Release(); x = nullptr;}

#define SafeDelete(x) {if(x) delete x; x = nullptr;}

template<typename T> 
inline void Clamp(T &x, T a, T b)
{
	x = (x < a) ? a : (x > b) ? b : x;
}

int32_t InvModGcdex(int32_t x, int32_t domainSize);

inline void ShowErrorMessage(LPWSTR file, int line)
{
	WCHAR errorMsg[500];
	swprintf_s(errorMsg, L"Problem in file %ls in line %d!", file, line);

	MessageBox(nullptr, errorMsg, L"Error!", MB_OK);
}

class DXException
{
public:
	DXException(HRESULT hr,	const std::wstring& funcName, const std::wstring& filename, int32_t line);
	std::wstring ToString() const;

private:
	HRESULT      mErrorCode;
	std::wstring mFuncName;
	std::wstring mFilename;
	int32_t      mLineNumber;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                   \
{                                                          \
	HRESULT __hr = (x);                                    \
	if(FAILED(__hr))                                       \
	{                                                      \
		throw DXException(__hr, L#x, __FILEW__, __LINE__); \
	}                                                      \
}
#endif

#define EXPECTED_WND_SIZE 900.0f //Approximate size of window

#define MINIMUM_FIELD_SIZE 1
#define MAXIMUM_FIELD_SIZE 175

#endif