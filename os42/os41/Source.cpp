#include<Windows.h>
#include <strsafe.h>
#include"resource.h"
#include <WindowsX.h>
#include <Shlobj_core.h>
#include <string>
#include<TCHAR.h>

#define CHOOSE_FILE_BUTTON 1
#define RENAME_BUTTON 2
#define CHANGE_BUTTON 3
#define CHOOSE_FOLDER_BUTTON 4
#define ERROR_RECEIVING_ATTRIBUTES "Ошибка. Невозможно получить данные"
#define IDC_CHECK1                      1003
#define IDC_CHECK2                      1004
#define IDC_CHECK3                      1005
#define IDC_CHECK4                      1006

HINSTANCE handle;
HWND hWnd = NULL, hName, hAttributes, hSize, hTimeCreating, hLastTimeEditing, hLastTimeUsing;
DWORD windowWidth, windowHeight, windowX, windowY;
WIN32_FILE_ATTRIBUTE_DATA win32_file_attribute_data;
CHAR fileName[MAX_PATH], newFileName[MAX_PATH];

VOID CreatingElements(HINSTANCE hInstance, HWND hWnd)
{
	CreateWindow("static", "Имя", WS_CHILD | WS_VISIBLE, 10, 10, 130, 30, hWnd, (HMENU)1, hInstance, NULL);
	hName = CreateWindow("static", "", WS_CHILD | WS_VISIBLE, 140, 10, 570, 30, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("static", "Атрибуты", WS_CHILD | WS_VISIBLE, 10, 40, 130, 30+100, hWnd, (HMENU)1, hInstance, NULL);
	hAttributes = CreateWindow("static", "", WS_CHILD | WS_VISIBLE, 140, 40, 570, 30+100, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("static", "Размер", WS_CHILD | WS_VISIBLE, 10, 70+100, 130, 30, hWnd, (HMENU)1, hInstance, NULL);
	hSize = CreateWindow("static", "", WS_CHILD | WS_VISIBLE, 140, 70+100, 570, 30, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("static", "Время создания", WS_CHILD | WS_VISIBLE, 10, 100+100, 130, 30, hWnd, (HMENU)1, hInstance, NULL);
	hTimeCreating = CreateWindow("static", "", WS_CHILD | WS_VISIBLE, 140, 100+100, 570, 30, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("static", "Время изменения", WS_CHILD | WS_VISIBLE, 10, 130+100, 130, 30, hWnd, (HMENU)1, hInstance, NULL);
	hLastTimeEditing = CreateWindow("static", "", WS_CHILD | WS_VISIBLE, 140, 130+100, 570, 30, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("static", "Время последнего обращения", WS_CHILD | WS_VISIBLE, 10, 160+100, 130, 40, hWnd, (HMENU)1, hInstance, NULL);
	hLastTimeUsing = CreateWindow("static", "", WS_CHILD | WS_VISIBLE, 140, 160+100, 570, 40, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("BUTTON", "Выбрать файл", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 50 , 220+100, 150, 50, hWnd, (HMENU)CHOOSE_FILE_BUTTON, hInstance, NULL);
	CreateWindow("BUTTON", "Выбрать каталог", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 210, 220+100, 150, 50, hWnd, (HMENU)CHOOSE_FOLDER_BUTTON, hInstance, NULL);
	CreateWindow("BUTTON", "Переименовать", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 220+100, 150, 50, hWnd, (HMENU)RENAME_BUTTON, hInstance, NULL);
	CreateWindow("BUTTON", "Изменить атрибуты", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 530, 220+100, 150, 50, hWnd, (HMENU)CHANGE_BUTTON, hInstance, NULL);
}

BOOL GetFileTimeFormat(const LPFILETIME lpFileTime, LPTSTR lpszFileTime, DWORD cchFileTime){
	SYSTEMTIME st;
	BOOL bRet = FileTimeToSystemTime(lpFileTime, &st);
	if (FALSE != bRet)
		bRet = SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
	if (FALSE != bRet)
	{
		GetDateFormat(LOCALE_USER_DEFAULT,DATE_LONGDATE, &st, NULL,lpszFileTime, cchFileTime);
		StringCchCat(lpszFileTime, cchFileTime, TEXT(", "));
		DWORD len = _tcslen(lpszFileTime);
		if (len < cchFileTime)
			GetTimeFormat(LOCALE_USER_DEFAULT,TIME_FORCE24HOURFORMAT, &st, NULL,lpszFileTime+len, cchFileTime-len);
	}
	return bRet;
}

VOID ClearingFileName(BOOL whatToClear) {
	if (whatToClear) {
		INT l;
		for (INT i = 0; i < MAX_PATH; i++)
			if (fileName[i] == '\0') {
				l = i;
				break;
			}
		for (INT i = l; i < MAX_PATH; i++)
			fileName[i] = '\0';
	}
	else {
		INT l;
		for (INT i = 0; i < MAX_PATH; i++)
			if (newFileName[i] == '\0') {
				l = i;
				break;
			}
		for (INT i = l; i < MAX_PATH; i++)
			newFileName[i] = '\0';
	}
}

BOOL CALLBACK NewNameDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwndDlg, WM_SETTEXT, 0, LPARAM(TEXT("Новое имя")));
		return TRUE;
	case WM_COMMAND: {
		if (LOWORD(wParam) == IDOK) {
			HWND hDialog = GetActiveWindow();
			GetDlgItemText(hDialog, IDHELP, newFileName, _countof(newFileName));
			ClearingFileName(FALSE);
			EndDialog(hwndDlg, 0);
		}
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwndDlg, 1);
		}
		return TRUE;
	}
	}
	return FALSE;
}

VOID LoadAttributes() {
	if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &win32_file_attribute_data) != FALSE) {
		std::string str = "";
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			str += "FILE_ATTRIBUTE_ARCHIVE\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
			str += "FILE_ATTRIBUTE_COMPRESSED\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
			str += "FILE_ATTRIBUTE_ENCRYPTED\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			str += "FILE_ATTRIBUTE_HIDDEN\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
			str += "FILE_ATTRIBUTE_NOT_CONTENT_INDEXED\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			str += "FILE_ATTRIBUTE_READONLY\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			str += "FILE_ATTRIBUTE_SYSTEM\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
			str += "FILE_ATTRIBUTE_TEMPORARY\n";
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			str += "FILE_ATTRIBUTE_DIRECTORY\n";
		}
		if (str.length() == 0)
			str = "Атрибутов нет";
		CHAR temp[MAX_PATH] = "";
		for (INT i = 0; i < str.length(); i++) {
			temp[i] = str[i];
		}
		SendMessage(hAttributes, WM_SETTEXT, 0, (LPARAM)temp);
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			StringCchPrintf(temp, 10, TEXT("- - - - -"));
		}
		else {
			StringCchPrintf(temp, 25, TEXT("%u байт"), win32_file_attribute_data.nFileSizeLow);
		}
		SendMessage(hSize, WM_SETTEXT, 0, (LPARAM)temp);
		GetFileTimeFormat(&win32_file_attribute_data.ftCreationTime, temp, _countof(temp));
		SendMessage(hTimeCreating, WM_SETTEXT, 0, (LPARAM)temp);
		GetFileTimeFormat(&win32_file_attribute_data.ftLastAccessTime, temp, _countof(temp));
		SendMessage(hLastTimeUsing, WM_SETTEXT, 0, (LPARAM)temp);
		GetFileTimeFormat(&win32_file_attribute_data.ftLastWriteTime, temp, _countof(temp));
		SendMessage(hLastTimeEditing, WM_SETTEXT, 0, (LPARAM)temp);
	}
	else {
		SendMessage(hAttributes, WM_SETTEXT, 0, (LPARAM)ERROR_RECEIVING_ATTRIBUTES);
		SendMessage(hSize, WM_SETTEXT, 0, (LPARAM)ERROR_RECEIVING_ATTRIBUTES);
		SendMessage(hTimeCreating, WM_SETTEXT, 0, (LPARAM)ERROR_RECEIVING_ATTRIBUTES);
		SendMessage(hLastTimeEditing, WM_SETTEXT, 0, (LPARAM)ERROR_RECEIVING_ATTRIBUTES);
		SendMessage(hLastTimeUsing, WM_SETTEXT, 0, (LPARAM)ERROR_RECEIVING_ATTRIBUTES);
	}
}

BOOL CALLBACK ChangeAttributesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG: {
		SendMessage(hwndDlg, WM_SETTEXT, 0, LPARAM(TEXT("Атрибуты")));
		CheckDlgButton(hwndDlg, IDC_CHECK1, 0);
		CheckDlgButton(hwndDlg, IDC_CHECK2, 0);
		CheckDlgButton(hwndDlg, IDC_CHECK3, 0);
		CheckDlgButton(hwndDlg, IDC_CHECK4, 0);
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			CheckDlgButton(hwndDlg, IDC_CHECK1, 1);
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			CheckDlgButton(hwndDlg, IDC_CHECK2, 1);
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
			CheckDlgButton(hwndDlg, IDC_CHECK4, 1);
		}
		if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			CheckDlgButton(hwndDlg, IDC_CHECK3, 1);
		}
		return TRUE;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == IDOK) {
			if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &win32_file_attribute_data) != FALSE) {
				DWORD fileAttributes = 0;
				if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
					fileAttributes |= FILE_ATTRIBUTE_SYSTEM;
				}
				if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
					fileAttributes |= FILE_ATTRIBUTE_TEMPORARY;
				}
				if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
					fileAttributes |= FILE_ATTRIBUTE_COMPRESSED;
				}
				if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
					fileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED) {
					fileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK2) == BST_CHECKED) {
					fileAttributes |= FILE_ATTRIBUTE_HIDDEN;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK4) == BST_CHECKED) {
					fileAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK3) == BST_CHECKED) {
					fileAttributes |= FILE_ATTRIBUTE_READONLY;
				}
				if (SetFileAttributes(fileName, fileAttributes) == FALSE) {
					MessageBox(hWnd, TEXT("Ошибка установки атрибутов!"), NULL, MB_OK | MB_ICONERROR);
				}
			}
			else {
				MessageBox(hWnd, TEXT("Ошибка установки атрибутов!"), NULL, MB_OK | MB_ICONERROR);
			}
			LoadAttributes();
			EndDialog(hwndDlg, 0);
		}
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwndDlg, 0);
		}
		return TRUE;
	}
	}
	return FALSE;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_CLOSE: {
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	case WM_QUIT: {
		DestroyWindow(hWnd);
		return 0;
	}
	case WM_CREATE:{
		CreatingElements(handle, hWnd);
		SetWindowText(hWnd, fileName);
		SendMessage(hName, WM_SETTEXT, 0, (LPARAM)fileName);
		LoadAttributes();
		return 0;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == CHOOSE_FILE_BUTTON) {
			OPENFILENAME ofn = { sizeof(OPENFILENAME) };
			ofn.hInstance = handle;
			ofn.lpstrFilter = TEXT("Все файлы");
			ofn.lpstrFile = fileName;
			ofn.nMaxFile = _countof(fileName);
			ofn.lpstrTitle = TEXT("Выбрать файл");
			ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn) != FALSE) {
				SetWindowText(hWnd, fileName);
				SendMessage(hName, WM_SETTEXT, 0, (LPARAM)fileName);
				LoadAttributes();
			}
		}
		if (LOWORD(wParam) == CHOOSE_FOLDER_BUTTON) {
			BROWSEINFO bi = { hWnd, NULL, NULL, "Выбрать папку", BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_BROWSEFORCOMPUTER, NULL, NULL, 0 };
			LPCITEMIDLIST lpItemDList = SHBrowseForFolder(&bi);
			if (lpItemDList != NULL){
				SHGetPathFromIDList(lpItemDList, (LPSTR)fileName);
				SetWindowText(hWnd, fileName);
				SendMessage(hName, WM_SETTEXT, 0, (LPARAM)fileName);
				LoadAttributes();
			}
		}
		if (LOWORD(wParam) == RENAME_BUTTON) {
			if (DialogBox(handle, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, NewNameDialogProc) == 0) {
				if (newFileName[0] != '\0') {
					CHAR extension[10];
					for (INT i = 0; i < 10; i++)
						extension[i] = '\0';
					for (INT i = MAX_PATH - 1; i >= 0; i--) {
						if ((fileName[i]) == '.') {
							INT j = 0;
							while (j < 10) {
								i++;
								extension[j] = fileName[i];
								j++;
							}
							break;
						}
					}
					INT index;
					for (INT i = MAX_PATH - 1; i >= 0; i--) {
						if (fileName[i] == '\\') {
							index = i;
							break;
						}
					}
					CHAR newName[MAX_PATH];
					for (INT i = 0; i < MAX_PATH; i++) {
						newName[i] = '\0';
					}
					for (INT i = 0; i < index + 1; i++)
						newName[i] = fileName[i];
					INT j = 0;
					for (INT i = index + 1; i < MAX_PATH; i++) {
						if (newFileName[j] != '\0') {
							newName[i] = newFileName[j];
							j++;
						}
						else {
							index = i;
							break;
						}
					}
					if (extension[0] != '\0') {
						newName[index] = '.';
						index++;
						for (INT i = 0; i < 10; i++)
							newName[i + index] = extension[i];
					}
					if (NULL == MoveFile(fileName, newName)) {
						MessageBox(hWnd, TEXT("Ошибка. Попробуйте другое имя!"), NULL, MB_OK | MB_ICONERROR);
					}
					else {
						for (INT i = 0; i < MAX_PATH; i++)
							fileName[i] = newName[i];
						SetWindowText(hWnd, fileName);
						SendMessage(hName, WM_SETTEXT, 0, (LPARAM)fileName);
						LoadAttributes();
					}
				}
				else {
					MessageBox(hWnd, TEXT("Ошибка. Имя не может быть пустым!"), NULL, MB_OK | MB_ICONERROR);

				}
				
			}
		}
		if (LOWORD(wParam) == CHANGE_BUTTON) {
			DialogBox(handle, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, ChangeAttributesDialogProc);
		}
		return 0;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

WNDCLASSEX RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX window = { sizeof(WNDCLASSEX) };
	window.style = CS_HREDRAW | CS_VREDRAW;
	window.lpfnWndProc = WindowProcedure;
	window.hInstance = hInstance;
	window.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window.lpszMenuName = NULL;
	window.lpszClassName = TEXT("WindowClass");
	window.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	return window;
}

VOID MessagesProcessing() {
	MSG  msg;
	BOOL bRet;
	while (bRet = GetMessage(&msg, NULL, 0, 0) != FALSE)
	{
		if (bRet != -1)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

VOID SaveRegistry() {
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\IT-32"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		RegSetValueEx(hKey, TEXT("WindowHeight"), 0, REG_DWORD, (LPCBYTE)&windowHeight, sizeof(DWORD));
		RegSetValueEx(hKey, TEXT("WindowWidth"), 0, REG_DWORD, (LPCBYTE)&windowWidth, sizeof(DWORD));
		RECT rect;
		GetWindowRect(hWnd,&rect);
		windowX = rect.left;
		windowY = rect.top;
		ClearingFileName(TRUE);
		RegSetValueEx(hKey, TEXT("WindowX"), 0, REG_DWORD, (LPCBYTE)&windowX, sizeof(DWORD));
		RegSetValueEx(hKey, TEXT("WindowY"), 0, REG_DWORD, (LPCBYTE)&windowY, sizeof(DWORD));
		RegSetValueEx(hKey, TEXT("FileName"), 0, REG_SZ, (LPCBYTE)&fileName, sizeof(fileName));
		RegCloseKey(hKey);
	}
}

VOID StandardParameters() {
	windowHeight = 450;  
	windowWidth = 740; 
	windowX = 100;
	windowY = 100;
	CHAR temp[MAX_PATH] = "C:\\Users\\VD\\Desktop";
	for (INT i = 0; i < MAX_PATH; i++)
		fileName[i] = temp[i];
	ClearingFileName(TRUE);
}

VOID LoadRegistry() {
	HKEY hKey;
	LPDWORD size = new DWORD;
	*size = sizeof(DWORD);
	LPDWORD sizeFile = new DWORD;
	*sizeFile = sizeof(fileName);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\IT-32"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		if ((RegQueryValueEx(hKey, TEXT("FileName"), 0, NULL, (LPBYTE)&fileName, sizeFile) != ERROR_SUCCESS) || (RegQueryValueEx(hKey, TEXT("WindowWidth"), 0, NULL, (LPBYTE)&windowWidth, size) != ERROR_SUCCESS) || (RegQueryValueEx(hKey, TEXT("WindowHeight"), 0, NULL, (LPBYTE)&windowHeight, size) != ERROR_SUCCESS) || (RegQueryValueEx(hKey, TEXT("WindowX"), 0, NULL, (LPBYTE)&windowX, size) != ERROR_SUCCESS) || (RegQueryValueEx(hKey, TEXT("WindowY"), 0, NULL, (LPBYTE)&windowY, size) != ERROR_SUCCESS)) {
			StandardParameters();
		}
		RegCloseKey(hKey);
	}
	ClearingFileName(TRUE);
	delete[]size;
	delete[]sizeFile;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	handle = hInstance;
	if (!RegisterClassEx(&RegisterWindowClass(hInstance)))
		return EXIT_FAILURE;
	LoadLibrary(TEXT("ComCtl32.dll"));
	LoadRegistry();
	hWnd = CreateWindowEx(0, TEXT("WindowClass"), TEXT(""), WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
		return EXIT_FAILURE;
	ShowWindow(hWnd, nCmdShow);
	MessagesProcessing();
	SaveRegistry();
	return EXIT_SUCCESS;
}