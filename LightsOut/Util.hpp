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