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

private:
	static bool FileSave(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const  COMDLG_FILTERSPEC *fileTypes, uint32_t fileTypesSize);

	static bool FileDialogHandle(HWND wnd, std::wstring& filename, LPWSTR dialogTitle, const COMDLG_FILTERSPEC *fileTypes,
								 uint32_t fileTypesSize, const CLSID dialogType);
};

#endif FILEDIALOG_HPP