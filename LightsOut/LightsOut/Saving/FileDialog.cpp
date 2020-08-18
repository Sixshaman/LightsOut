#include "FileDialog.hpp"
#include "..\Util.hpp"
#include <KnownFolders.h>
#include <wrl\client.h>
#include <regex>

bool FileDialog::GetPictureToSave(HWND wnd, std::wstring& filename)
{
	const COMDLG_FILTERSPEC fileTypes[] =
	{
		{L"Pictures(*.bmp)", L"*.bmp;"},
		{L"All Files (*.*)", L"*.*"}
	};

	return FileDialogHandle(wnd, filename, L"Save Picture", fileTypes, ARRAYSIZE(fileTypes), L"bmp", CLSID_FileSaveDialog);
}

bool FileDialog::GetFolderToSave(HWND wnd, std::wstring& filename)
{
	return FolderDialogHandle(wnd, filename, L"Select folder");
}

bool FileDialog::FileDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC* fileTypes, uint32_t fileTypesSize, const std::wstring& defaultFileType, const CLSID dialogType)
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

	std::wregex re(L".*\\." + defaultFileType); //Check if extension is present
	if(!std::regex_match(filename, re))
	{
		filename += (L"." + defaultFileType);
	}

	return true;
}

bool FileDialog::FolderDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle)
{
	Microsoft::WRL::ComPtr<IFileDialog> pFileDialog;
	ThrowIfFailed(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog)));

	DWORD options = 0;
	ThrowIfFailed(pFileDialog->GetOptions(&options));
	ThrowIfFailed(pFileDialog->SetOptions(options | FOS_PICKFOLDERS));

	Microsoft::WRL::ComPtr<IShellItem> pShellFolder;
	ThrowIfFailed(SHCreateItemInKnownFolder(FOLDERID_Pictures, 0, nullptr, IID_PPV_ARGS(&pShellFolder)));
	ThrowIfFailed(pFileDialog->SetFolder(pShellFolder.Get()));

	ThrowIfFailed(pFileDialog->SetTitle(dialogTitle));

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

	return true;
}
