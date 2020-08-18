#ifndef FILEDIALOG_HPP
#define FILEDIALOG_HPP

#include <Windows.h>
#include <string>
#include <ShObjIdl.h>

/*
 * Class for save file dialog. 
 *
 */

class FileDialog
{
public:
	static bool GetPictureToSave(HWND wnd, std::wstring& filename);

	static bool GetFolderToSave(HWND wnd, std::wstring& filename);

private:
	static bool FileDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC *fileTypes, uint32_t fileTypesSize, const std::wstring& defaultFileType, const CLSID dialogType);

	static bool FolderDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle);
};

#endif FILEDIALOG_HPP