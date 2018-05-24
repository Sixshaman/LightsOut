#ifndef UTIL_HPP
#define UTIL_HPP

#include <tchar.h>
#include <DirectXMath.h>

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

inline void ShowErrorMessage(LPWSTR file, int line)
{
	WCHAR errorMsg[500];
	swprintf_s(errorMsg, L"Problem in file %ls in line %d!", file, line);

	MessageBox(nullptr, errorMsg, L"Error!", MB_OK);
}

#define ASSERT_ERR(X) {if(FAILED(X)) {ShowErrorMessage(_T(__FILE__), __LINE__); return false;}}

#define EXPECTED_WND_SIZE 900.0f //Approximate size of window

#define MINIMUM_FIELD_SIZE 1
#define MAXIMUM_FIELD_SIZE 175

#endif