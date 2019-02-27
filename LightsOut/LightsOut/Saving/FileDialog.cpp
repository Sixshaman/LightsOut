#include "FileDialog.hpp"
#include "..\Util.hpp"
#include <KnownFolders.h>
#include <wrl\client.h>

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

bool FileDialog::FileSave(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC *fileTypes, uint32_t fileTypesSize)
{
	if(!FileDialogHandle(wnd, filename, dialogTitle, fileTypes, fileTypesSize, CLSID_FileSaveDialog))
	{
		return false;
	}

	return true;
}

bool FileDialog::FileDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC *fileTypes, uint32_t fileTypesSize, const CLSID dialogType)
{
	Microsoft::WRL::ComPtr<IFileDialog> pFileDialog;
	ThrowIfFailed(CoCreateInstance(dialogType, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog)));

	if(dialogType == CLSID_FileOpenDialog)
	{
		Microsoft::WRL::ComPtr<IShellItem> pShellFolder;
		ThrowIfFailed(SHCreateItemInKnownFolder(FOLDERID_Pictures, 0, nullptr, IID_PPV_ARGS(&pShellFolder)));
		ThrowIfFailed(pFileDialog->SetFolder(pShellFolder.Get()));
	}

	ThrowIfFailed(pFileDialog->SetTitle(dialogTitle));
	ThrowIfFailed(pFileDialog->SetFileTypes(fileTypesSize, fileTypes));
	ThrowIfFailed(pFileDialog->SetFileTypeIndex(1));

	if(dialogType == CLSID_FileSaveDialog)
	{
		DWORD options = 0;
		ThrowIfFailed(pFileDialog->GetOptions(&options));
		ThrowIfFailed(pFileDialog->SetOptions(options | FOS_OVERWRITEPROMPT));
	}

	HRESULT hr = pFileDialog->Show(wnd);
	if(hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{	
		ThrowIfFailed(hr);
	}
	else
	{
		return false;
	}

	Microsoft::WRL::ComPtr<IShellItem> pShellResult;
	ThrowIfFailed(pFileDialog->GetResult(&pShellResult));

	PWSTR filePath = nullptr;
	ThrowIfFailed(pShellResult->GetDisplayName(SIGDN_FILESYSPATH, &filePath));

	filename = std::wstring(filePath);
	CoTaskMemFree(filePath);

	if(filename.size() <= 3 || filename.substr(filename.length() - 4, 4) != L".bmp")
	{
		filename += L".bmp";
	}

	return true;
}