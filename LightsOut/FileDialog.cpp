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

#include "FileDialog.hpp"
#include "Util.hpp"
#include <KnownFolders.h>

bool FileDialog::GetPictureToSave(HWND wnd, std::wstring& filename)
{
	const COMDLG_FILTERSPEC fileTypes[] =
	{
		{L"Pictures(*.bmp)", L"*.bmp;"},
		{L"All Files (*.*)", L"*.*"}
	};

	if(!FileSave(wnd, filename, L"Save Picture", fileTypes, ARRAYSIZE(fileTypes)))
	{
		return false;
	}

	return true;
}

bool FileDialog::FileSave(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC *fileTypes, UINT fileTypesSize)
{
	if(!FileDialogHandle(wnd, filename, dialogTitle, fileTypes, fileTypesSize, CLSID_FileSaveDialog))
	{
		return false;
	}

	return true;
}

bool FileDialog::FileDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC *fileTypes,
								  UINT fileTypesSize, const CLSID dialogType)
{
	IFileDialog *pFileDialog = nullptr;																	
	if(FAILED(CoCreateInstance(dialogType, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog)))) 
	{
		MessageBox(nullptr, L"Cannot create file dialog instance", L"Error", MB_OK);
		return false;
	}

	if(dialogType == CLSID_FileOpenDialog)
	{
		IShellItem *shellFolder = nullptr;															
		if(FAILED(SHCreateItemInKnownFolder(FOLDERID_Pictures, 0, nullptr, IID_PPV_ARGS(&shellFolder))))
		{
			SafeRelease(pFileDialog);
			SafeRelease(shellFolder);

			MessageBox(nullptr, L"Cannot create shell item", L"Error", MB_OK);
			return false;
		}

		if(FAILED(pFileDialog->SetFolder(shellFolder)))
		{
			SafeRelease(pFileDialog);
			SafeRelease(shellFolder);

			MessageBox(nullptr, L"Cannot set folder!", L"Error", MB_OK);
			return false;
		}

		SafeRelease(shellFolder);
	}

	if(FAILED(pFileDialog->SetTitle(dialogTitle)))
	{
		SafeRelease(pFileDialog);

		MessageBox(nullptr, L"Cannot set title!", L"Error", MB_OK);
		return false;
	}

	if(FAILED(pFileDialog->SetFileTypes(fileTypesSize, fileTypes))) 
	{
		SafeRelease(pFileDialog);

		MessageBox(nullptr, L"Cannot set file types!", L"Error", MB_OK);
		return false;
	}

	if(FAILED(pFileDialog->SetFileTypeIndex(1)))
	{
		SafeRelease(pFileDialog);

		MessageBox(nullptr, L"Cannot set file type index!", L"Error", MB_OK);
		return false;
	}

	if(dialogType == CLSID_FileSaveDialog)
	{
		DWORD options;
		if(FAILED(pFileDialog->GetOptions(&options)))
		{
			SafeRelease(pFileDialog);

			MessageBox(nullptr, L"Cannot get options!", L"Error", MB_OK);
			return false;
		}

		if(FAILED(pFileDialog->SetOptions(options | FOS_OVERWRITEPROMPT))) 
		{
			SafeRelease(pFileDialog);

			MessageBox(nullptr, L"Cannot set options!", L"Error", MB_OK);
			return false;
		}
	}

	HRESULT hr = pFileDialog->Show(wnd);
	if(FAILED(hr))
	{
		SafeRelease(pFileDialog);
		
		if(hr != 2147943623)
		{
			MessageBox(nullptr, L"Cannot show file dialog!", L"Error", MB_OK);
		}

		return false;
	}

	IShellItem *shellResult = nullptr;
	if(FAILED(pFileDialog->GetResult(&shellResult)))
	{
		SafeRelease(pFileDialog);

		MessageBox(nullptr, L"Cannot get result!", L"Error", MB_OK);
		return false;
	}

	PWSTR filePath = nullptr;
	if(FAILED(shellResult->GetDisplayName(SIGDN_FILESYSPATH, &filePath)))
	{
		SafeRelease(shellResult);
		SafeRelease(pFileDialog);

		MessageBox(nullptr, L"Cannot get file name!", L"Error", MB_OK);
		return false;
	}

	filename = std::wstring(filePath);
	CoTaskMemFree(filePath);

	SafeRelease(shellResult);
	SafeRelease(pFileDialog);

	return true;
}