#include<Windows.h>
#include <strsafe.h>
#include"resource.h"
#include <WindowsX.h>

#define TEXT_EDIT 1
#define MENU_EXIT_ID 2
#define MENU_SAVE_FILE 3
#define FONT_SIZE_20 4
#define FONT_SIZE_25 5
#define FONT_SIZE_30 6
#define FONT_STYLE_BOLD 7
#define FONT_STYLE_STANDARD 8
#define FONT_STYLE_ITALIC 9
#define MENU_NEW_FILE 12
#define MENU_SAVE_FILE 13
#define MENU_SAVE_AS 14
#define MENU_OPEN_FILE 15
#define SAVING_CANCEL 666

HINSTANCE handle;
HWND hWnd = NULL,hTextEdit;
int windowWidth, windowHeight, windowX, windowY;
HMENU hMenuBar;
HFONT hFont;
LOGFONT logFont;
TCHAR fileNameHeading[MAX_PATH], initializationFileName[MAX_PATH]=TEXT("D:\\app.ini"), fileName[MAX_PATH];
LPSTR lpszBufferText = NULL;
BOOL wasEdit = FALSE;
HANDLE hFile;
OVERLAPPED ReadStructure = { 0 }, WriteStructure = { 0 };
DWORD sizeRead;

BOOL ReadAsync(LPVOID lpBuffer, DWORD dwOffset, DWORD dwSize, LPOVERLAPPED lpOverlapped)
{
	ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));
	lpOverlapped->Offset = dwOffset;
	lpOverlapped->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	BOOL bRet = ReadFile(hFile, lpBuffer, dwSize, NULL, lpOverlapped);
	if (FALSE == bRet && ERROR_IO_PENDING != GetLastError())
	{
		CloseHandle(lpOverlapped->hEvent), lpOverlapped->hEvent = NULL;
		return FALSE;
	}
	return TRUE;
} 

BOOL WriteAsync(HANDLE hFile, LPCVOID lpBuffer, DWORD dwOffset, DWORD dwSize, LPOVERLAPPED lpOverlapped)
{
	ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));
	lpOverlapped->Offset = dwOffset;
	lpOverlapped->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	BOOL bRet = WriteFile(hFile, lpBuffer, dwSize, NULL, lpOverlapped);
	if (FALSE == bRet && ERROR_IO_PENDING != GetLastError())
	{
		CloseHandle(lpOverlapped->hEvent), lpOverlapped->hEvent = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL FinishIo(LPOVERLAPPED lpOverlapped)
{
	if (NULL != lpOverlapped->hEvent)
	{
		WaitForSingleObject(lpOverlapped->hEvent, INFINITE);
			CloseHandle(lpOverlapped->hEvent), lpOverlapped->hEvent = NULL;
			return TRUE;
	}
	return FALSE;
}

BOOL OpenFile()
{
	HANDLE hExistingFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (INVALID_HANDLE_VALUE == hExistingFile)
	{
		return FALSE;
	}
	Edit_SetText(hTextEdit, NULL);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		FinishIo(&WriteStructure);
		CloseHandle(hFile);
	} 
	hFile = hExistingFile;
	LARGE_INTEGER size;
	BOOL bRet = GetFileSizeEx(hFile, &size);
	sizeRead = size.LowPart;
	if ((FALSE != bRet) && (size.LowPart > 0))
	{
		lpszBufferText = new CHAR[size.LowPart];
		bRet = ReadAsync( lpszBufferText, 0, size.LowPart, &ReadStructure);
		if (FALSE == bRet)
		{
			delete[] lpszBufferText, lpszBufferText = NULL;
		}
	}
	return bRet;
}

BOOL SaveFile(BOOL SaveAs)
{
	if (SaveAs)
	{
		HANDLE hNewFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
		if (INVALID_HANDLE_VALUE == hNewFile)
			return FALSE;
		if (INVALID_HANDLE_VALUE != hFile)
		{
			FinishIo(&WriteStructure);
			CloseHandle(hFile);
		}
		hFile = hNewFile;
	}
	else 
		if (INVALID_HANDLE_VALUE != hFile)
			FinishIo(&WriteStructure);
		else
		{
			hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
			if (INVALID_HANDLE_VALUE == hFile)
				return FALSE;
		}
	LARGE_INTEGER size;
	size.QuadPart = GetWindowTextLengthA(hTextEdit);
	BOOL bRet = SetFilePointerEx(hFile, size, NULL, FILE_BEGIN);
	if (FALSE != bRet)
		bRet = SetEndOfFile(hFile);
	if ((FALSE != bRet) && (size.LowPart > 0))
	{
		lpszBufferText = new CHAR[size.LowPart+1];
		GetWindowTextA(hTextEdit, lpszBufferText, size.LowPart+1);
		bRet = WriteAsync(hFile, lpszBufferText, 0, size.LowPart, &WriteStructure);
		if (FALSE == bRet)
		{
			delete[] lpszBufferText, lpszBufferText = NULL;
		}
	}
	if (TRUE == bRet)
		wasEdit = FALSE;
	return bRet;
}

VOID CutHeading()
{
	TCHAR temp[MAX_PATH] = TEXT("");
	for (INT i = 0; i < _countof(fileNameHeading); i++) {
		if (fileNameHeading[i] != TEXT('.')) {
			temp[i] = fileNameHeading[i];
		}
		else {
			break;
		}
	}
	for (INT i = 0; i < _countof(fileNameHeading); i++)
		 fileNameHeading[i]= temp[i];
}

VOID CreatingElements(HINSTANCE hInstance, HWND hWnd)
{
	hTextEdit = CreateWindowEx(0, TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_WANTRETURN, 0, 0, windowWidth-17, windowHeight-40, hWnd, (HMENU)TEXT_EDIT, hInstance, NULL);
	CutHeading();
	SetWindowText(hWnd, fileNameHeading);
}

VOID ErrorSaving() {
	MessageBox(NULL, TEXT("Не удалось сохранить текстовый файл"), NULL, MB_OK | MB_ICONERROR);
}

VOID ErrorOpening() {
	MessageBox(NULL, TEXT("Не удалось открыть текстовый файл"), NULL, MB_OK | MB_ICONERROR);
}

BOOL CALLBACK SavingConfirmation(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwndDlg, WM_SETTEXT, 0, LPARAM(TEXT("Сохранение")));
		SetDlgItemText(hwndDlg, IDHELP, TEXT("Сохранить файл?"));
		SetDlgItemText(hwndDlg, IDC_BUTTON1, TEXT("Отмена"));
		SetDlgItemText(hwndDlg, IDC_BUTTON2, TEXT("Сохранить"));
		SetDlgItemText(hwndDlg, IDC_BUTTON3, TEXT("Не сохранять"));
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON1)
			EndDialog(hwndDlg,0);
		if (LOWORD(wParam) == IDC_BUTTON3)
			EndDialog(hwndDlg, SAVING_CANCEL);
		if (LOWORD(wParam) == IDC_BUTTON2)
			if ((fileNameHeading[0] == 'Б') && (fileNameHeading[9] == 'й') && (fileNameHeading[3] == 'ы')) {
				OPENFILENAME ofn = { sizeof(OPENFILENAME) };
				ofn.hInstance = handle;
				ofn.lpstrFilter = TEXT("Текстовые документы (*.txt)\0*.txt\0");
				ofn.lpstrFile = fileName;
				ofn.lpstrFileTitle = fileNameHeading;
				ofn.nMaxFile = _countof(fileName);
				ofn.nMaxFileTitle = _countof(fileNameHeading);
				ofn.lpstrTitle = TEXT("Сохранить");
				ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
				ofn.lpstrDefExt = TEXT("txt");
				if ((GetSaveFileName(&ofn) == FALSE) || (SaveFile(TRUE) == FALSE))
					ErrorSaving();
				EndDialog(hwndDlg, 0);
			}
			else {
				if (SaveFile(FALSE) == FALSE)
					ErrorSaving();
				EndDialog(hwndDlg, 0);
			}
		CutHeading();
		SetWindowText(hWnd, fileNameHeading);
		return TRUE;
	}
	return FALSE;
}

VOID MenuInitializer(HWND hwnd) {
	hMenuBar = CreateMenu(); 
	HMENU hFile = CreateMenu(); 
	HMENU hFont = CreateMenu(); 
	HMENU hFontStyle = CreateMenu();
	HMENU hFontBold = CreateMenu();
	HMENU hFontItalic = CreateMenu();
	HMENU hFontStandard = CreateMenu();
	HMENU hFontSize = CreateMenu();
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, TEXT("Файл")); 
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFont, TEXT("Шрифт"));
	AppendMenu(hFont, MF_POPUP, (UINT_PTR)hFontStyle, TEXT("Начертание"));
	AppendMenu(hFont, MF_POPUP, (UINT_PTR)hFontSize, TEXT("Размер"));
	AppendMenu(hFontSize, MF_STRING, FONT_SIZE_20, TEXT("20"));
	AppendMenu(hFontSize, MF_STRING, FONT_SIZE_25, TEXT("25"));
	AppendMenu(hFontSize, MF_STRING, FONT_SIZE_30, TEXT("30"));
	AppendMenu(hFontStyle, MF_STRING, FONT_STYLE_BOLD, TEXT("Полужирный"));
	AppendMenu(hFontStyle, MF_STRING , FONT_STYLE_STANDARD, TEXT("Обычный"));
	AppendMenu(hFontStyle, MF_STRING, FONT_STYLE_ITALIC, TEXT("Курсив"));
	AppendMenu(hFile, MF_STRING, MENU_NEW_FILE, TEXT("Новый"));
	AppendMenu(hFile, MF_STRING, MENU_OPEN_FILE, TEXT("Открыть"));
	AppendMenu(hFile, MF_STRING, MENU_SAVE_FILE, TEXT("Сохранить"));
	AppendMenu(hFile, MF_STRING, MENU_SAVE_AS, TEXT("Сохранить как"));
	AppendMenu(hFile, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFile, MF_STRING, MENU_EXIT_ID, TEXT("Выход")); 
	if(logFont.lfItalic)
		CheckMenuItem(hMenuBar, FONT_STYLE_ITALIC, MF_STRING | MF_CHECKED);
	if(logFont.lfWeight == FW_BOLD)
			CheckMenuItem(hMenuBar, FONT_STYLE_BOLD, MF_STRING | MF_CHECKED);
	if (logFont.lfWeight == FW_NORMAL)
		CheckMenuItem(hMenuBar, FONT_STYLE_STANDARD, MF_STRING | MF_CHECKED);
	if (logFont.lfHeight == 20)
		CheckMenuItem(hMenuBar, FONT_SIZE_20, MF_STRING | MF_CHECKED);
	if (logFont.lfHeight == 25)
		CheckMenuItem(hMenuBar, FONT_SIZE_25, MF_STRING | MF_CHECKED);
	if (logFont.lfHeight == 30)
		CheckMenuItem(hMenuBar, FONT_SIZE_30, MF_STRING | MF_CHECKED);
	SetMenu(hwnd, hMenuBar);
}

VOID SetNewMenuCheckBoxSize() {
	CheckMenuItem(hMenuBar, FONT_SIZE_20, MF_STRING | MF_UNCHECKED);
	CheckMenuItem(hMenuBar, FONT_SIZE_25, MF_STRING | MF_UNCHECKED);
	CheckMenuItem(hMenuBar, FONT_SIZE_30, MF_STRING | MF_UNCHECKED);
}

VOID SetNewMenuCheckBoxStyle() {
	CheckMenuItem(hMenuBar, FONT_STYLE_STANDARD, MF_STRING | MF_UNCHECKED);
	CheckMenuItem(hMenuBar, FONT_STYLE_BOLD, MF_STRING | MF_UNCHECKED);
	CheckMenuItem(hMenuBar, FONT_STYLE_ITALIC, MF_STRING | MF_UNCHECKED);
}

VOID MenuItalic() {
	if (logFont.lfItalic) 
		CheckMenuItem(hMenuBar, FONT_STYLE_ITALIC, MF_STRING | MF_CHECKED);
	else 
		CheckMenuItem(hMenuBar, FONT_STYLE_ITALIC, MF_STRING | MF_UNCHECKED);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg)
	{
	case WM_CLOSE: {
		if (!wasEdit)
			PostQuitMessage(EXIT_SUCCESS);
		else {
			if (DialogBox(handle, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, SavingConfirmation) == SAVING_CANCEL)
				PostQuitMessage(EXIT_SUCCESS);
		}
		return 0;
	}
	case WM_QUIT: {
		DestroyWindow(hWnd);
		return 0;
	}
	case WM_SIZE: {
		windowWidth = LOWORD(lParam);
		windowHeight = HIWORD(lParam);
		MoveWindow(hTextEdit, 0, 0, windowWidth, windowHeight, TRUE);
		return 0;
	}
	case WM_GETMINMAXINFO:{
		MINMAXINFO *pInfo = (MINMAXINFO *)lParam;
		POINT Min = { 300, 300 };
		pInfo->ptMinTrackSize = Min;
		return 0;
	}
	case WM_CREATE:{
		CreatingElements(handle, hWnd);
		MenuInitializer(hWnd);
		hFont = CreateFontIndirect(&logFont);
		SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		if (OpenFile() != FALSE) {
			if (sizeRead > 0) {
				LPSTR temp = new CHAR[sizeRead + 1];
				for (INT i = 0; i < sizeRead; i++)
					temp[i] = lpszBufferText[i];
				temp[sizeRead] = '\0';
				Edit_SetText(hTextEdit, temp);
				delete[]temp;
				temp = NULL;
				FinishIo(&WriteStructure);
				FinishIo(&ReadStructure);
				delete[] lpszBufferText;
				lpszBufferText = NULL;
				CutHeading();
			}
			else {
				Edit_SetText(hTextEdit, NULL);
				
			}
		}
		else {
			TCHAR temp[MAX_PATH] = TEXT("Безымянный");
			for (INT i = 0; i < MAX_PATH; i++)
				fileNameHeading[i] = temp[i];
			ErrorOpening();
		}
		SetWindowText(hWnd, fileNameHeading);
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == FONT_SIZE_20){
			SetNewMenuCheckBoxSize();
			CheckMenuItem(hMenuBar, FONT_SIZE_20, MF_STRING | MF_CHECKED);
			logFont.lfHeight = 20;
			logFont.lfWidth = 8;
			hFont = CreateFontIndirect(&logFont);
			SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		}
		if (LOWORD(wParam) == FONT_SIZE_25) {
			SetNewMenuCheckBoxSize();
			logFont.lfHeight = 25;
			logFont.lfWidth = 10;
			hFont = CreateFontIndirect(&logFont);
			CheckMenuItem(hMenuBar, FONT_SIZE_25, MF_STRING | MF_CHECKED);
			SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		}
		if (LOWORD(wParam) == FONT_SIZE_30) {
			SetNewMenuCheckBoxSize();
			CheckMenuItem(hMenuBar, FONT_SIZE_30, MF_STRING | MF_CHECKED);
			logFont.lfHeight = 30;
			logFont.lfWidth = 12;
			hFont = CreateFontIndirect(&logFont);
			SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		}
		if (LOWORD(wParam) == FONT_STYLE_STANDARD) {
			SetNewMenuCheckBoxStyle();
			CheckMenuItem(hMenuBar, FONT_STYLE_STANDARD, MF_STRING | MF_CHECKED);
			logFont.lfWeight = FW_NORMAL;
			hFont = CreateFontIndirect(&logFont);
			MenuItalic();
			SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		}
		if (LOWORD(wParam) == FONT_STYLE_BOLD) {
			SetNewMenuCheckBoxStyle();
			CheckMenuItem(hMenuBar, FONT_STYLE_BOLD, MF_STRING | MF_CHECKED);
			logFont.lfWeight = FW_BOLD;
			hFont = CreateFontIndirect(&logFont);
			MenuItalic();
			SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		}
		if (LOWORD(wParam) == FONT_STYLE_ITALIC) {
			if (!logFont.lfItalic) {
				logFont.lfItalic = TRUE;
			}
			else {
				logFont.lfItalic = FALSE;
			}
			MenuItalic();
			hFont = CreateFontIndirect(&logFont);
			SendMessage(hTextEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		}
		if (LOWORD(wParam) == MENU_SAVE_FILE) {
			if((fileNameHeading[0] == 'Б') && (fileNameHeading[9] == 'й') && (fileNameHeading[3] == 'ы')) {
				OPENFILENAME ofn = { sizeof(OPENFILENAME) };
				ofn.hInstance = handle;
				ofn.lpstrFilter = TEXT("Текстовые документы (*.txt)\0*.txt\0");
				ofn.lpstrFile = fileName;
				ofn.lpstrFileTitle = fileNameHeading;
				ofn.nMaxFile = _countof(fileName);
				ofn.nMaxFileTitle = _countof(fileNameHeading);
				ofn.lpstrTitle = TEXT("Сохранить");
				ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
				ofn.lpstrDefExt = TEXT("txt");
				if((GetSaveFileName(&ofn) == FALSE) || (SaveFile(TRUE) == FALSE))
					ErrorSaving();
			}
			else
				if (SaveFile(FALSE) == FALSE) 
					ErrorSaving();
		}
		if (LOWORD(wParam) == MENU_SAVE_AS) {
			OPENFILENAME ofn = { sizeof(OPENFILENAME) };
			ofn.hInstance = handle;
			ofn.lpstrFilter = TEXT("Текстовые документы (*.txt)\0*.txt\0");
			ofn.lpstrFile = fileName;
			ofn.nMaxFile = _countof(fileName);
			ofn.nMaxFileTitle = _countof(fileNameHeading);
			ofn.lpstrFileTitle = fileNameHeading;
			ofn.lpstrTitle = TEXT("Сохранить как");
			ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = TEXT("txt");
			if ((GetSaveFileName(&ofn) == FALSE) || (SaveFile(TRUE) == FALSE))
				ErrorSaving();
		}
		if (LOWORD(wParam) == MENU_NEW_FILE) {
			if (!wasEdit)
				Edit_SetText(hTextEdit, NULL);
			else {
				if (DialogBox(handle, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, SavingConfirmation) == SAVING_CANCEL) {
					wasEdit = FALSE;
					Edit_SetText(hTextEdit, NULL);
				}
			}
			TCHAR temp[MAX_PATH] = TEXT("Безымянный");
			for (INT i = 0; i < MAX_PATH; i++)
				fileNameHeading[i] = temp[i];
		}
		if (LOWORD(wParam) == MENU_OPEN_FILE) {
			if(wasEdit){
				if (DialogBox(handle, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, SavingConfirmation) == SAVING_CANCEL) {
					wasEdit = FALSE;
				}
			}
			OPENFILENAME ofn = { sizeof(OPENFILENAME) };
			ofn.hInstance = handle;
			ofn.lpstrFilter = TEXT("Текстовые документы (*.txt)\0*.txt\0");
			ofn.lpstrFile = fileName;
			ofn.nMaxFileTitle = _countof(fileNameHeading);
			ofn.nMaxFile = _countof(fileName);
			ofn.lpstrFileTitle = fileNameHeading;
			ofn.lpstrTitle = TEXT("Открыть");
			ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = TEXT("txt");
			if (GetOpenFileName(&ofn) != FALSE){
				if (OpenFile() != FALSE)
				{
					if (sizeRead > 0) {
						LPSTR temp = new CHAR[sizeRead + 1];
						for (INT i = 0; i < sizeRead; i++)
							temp[i] = lpszBufferText[i];
						temp[sizeRead] = '\0';
						Edit_SetText(hTextEdit, temp);
						delete[]temp;
						temp = NULL;
						FinishIo(&WriteStructure);
						FinishIo(&ReadStructure);
						delete[] lpszBufferText;
						lpszBufferText = NULL;
					}
					else {
						Edit_SetText(hTextEdit, NULL);
					}
					CutHeading();
					SetWindowText(hWnd, fileNameHeading);
				}
				else
					ErrorOpening();
			}
		}
		if (HIWORD(wParam) == EN_CHANGE) {
			wasEdit = TRUE;
		}
		if (LOWORD(wParam) == MENU_EXIT_ID) {
			if (!wasEdit)
				PostQuitMessage(EXIT_SUCCESS);
			else {
				if (DialogBox(handle, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, SavingConfirmation) == SAVING_CANCEL)
					PostQuitMessage(EXIT_SUCCESS);
			}
		}
		CutHeading();
		SetWindowText(hWnd, fileNameHeading);
		return 0;
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

VOID SaveInitializationFile() {
	WritePrivateProfileString(TEXT("File"), TEXT("Name"), fileName, initializationFileName);
	WritePrivateProfileString(TEXT("File"), TEXT("Heading"), fileNameHeading, initializationFileName);
	RECT rect;
	TCHAR tempString[5];
	GetWindowRect(hWnd, &rect);
	StringCchPrintf(tempString, 5, TEXT("%d"), rect.left);
	WritePrivateProfileString(TEXT("Window"), TEXT("X"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), rect.top);
	WritePrivateProfileString(TEXT("Window"), TEXT("Y"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), rect.right-rect.left);
	WritePrivateProfileString(TEXT("Window"), TEXT("Width"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), rect.bottom - rect.top);
	WritePrivateProfileString(TEXT("Window"), TEXT("Height"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfCharSet);
	WritePrivateProfileString(TEXT("FONT"), TEXT("CharSet"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfClipPrecision);
	WritePrivateProfileString(TEXT("FONT"), TEXT("ClipPrecision"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfEscapement);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Escapement"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfOrientation);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Orientation"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfOutPrecision);
	WritePrivateProfileString(TEXT("FONT"), TEXT("OutPrecision"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfItalic);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Italic"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfHeight);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Height"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfPitchAndFamily);
	WritePrivateProfileString(TEXT("FONT"), TEXT("PitchAndFamily"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfQuality);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Quality"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfStrikeOut);
	WritePrivateProfileString(TEXT("FONT"), TEXT("StrikeOut"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfUnderline);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Underline"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfWeight);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Weight"), tempString, initializationFileName);
	StringCchPrintf(tempString, 5, TEXT("%d"), logFont.lfWidth);
	WritePrivateProfileString(TEXT("FONT"), TEXT("Width"), tempString, initializationFileName);
}

VOID LoadInitializationFile() {
	GetPrivateProfileString(TEXT("File"), TEXT("Name"), NULL, fileName, MAX_PATH, initializationFileName);
	GetPrivateProfileString(TEXT("File"), TEXT("Heading"), NULL, fileNameHeading, MAX_PATH, initializationFileName);
	windowX = GetPrivateProfileInt(TEXT("Window"), TEXT("X"), NULL, initializationFileName);
	windowY = GetPrivateProfileInt(TEXT("Window"), TEXT("Y"), NULL, initializationFileName);
	windowWidth = GetPrivateProfileInt(TEXT("Window"), TEXT("Width"), NULL, initializationFileName);
	windowHeight = GetPrivateProfileInt(TEXT("Window"), TEXT("Height"), NULL, initializationFileName);
	logFont.lfCharSet = GetPrivateProfileInt(TEXT("Font"), TEXT("CharSet"), NULL, initializationFileName);
	logFont.lfClipPrecision = GetPrivateProfileInt(TEXT("Font"), TEXT("ClipPrecision"), NULL, initializationFileName);
	logFont.lfEscapement = GetPrivateProfileInt(TEXT("Font"), TEXT("Escapement"), NULL, initializationFileName);
	logFont.lfOrientation = GetPrivateProfileInt(TEXT("Font"), TEXT("Orientation"), NULL, initializationFileName);
	logFont.lfOutPrecision = GetPrivateProfileInt(TEXT("Font"), TEXT("OutPrecision"), NULL, initializationFileName);
	logFont.lfItalic = GetPrivateProfileInt(TEXT("Font"), TEXT("Italic"), NULL, initializationFileName);
	logFont.lfHeight = GetPrivateProfileInt(TEXT("Font"), TEXT("Height"), NULL, initializationFileName);
	logFont.lfPitchAndFamily = GetPrivateProfileInt(TEXT("Font"), TEXT("PitchAndFamily"), NULL, initializationFileName);
	logFont.lfQuality = GetPrivateProfileInt(TEXT("Font"), TEXT("Quality"), NULL, initializationFileName);
	logFont.lfStrikeOut = GetPrivateProfileInt(TEXT("Font"), TEXT("StrikeOut"), NULL, initializationFileName);
	logFont.lfUnderline = GetPrivateProfileInt(TEXT("Font"), TEXT("Underline"), NULL, initializationFileName);
	logFont.lfWeight = GetPrivateProfileInt(TEXT("Font"), TEXT("Weight"), NULL, initializationFileName);
	logFont.lfWidth = GetPrivateProfileInt(TEXT("Font"), TEXT("Width"), NULL, initializationFileName);
}

VOID StandardParameters() {
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logFont.lfItalic = FALSE;
	logFont.lfHeight = 25;
	logFont.lfPitchAndFamily = VARIABLE_PITCH;
	logFont.lfQuality = DEFAULT_QUALITY;
	logFont.lfStrikeOut = FALSE;
	logFont.lfUnderline = FALSE;
	logFont.lfWeight = FW_NORMAL;
	logFont.lfWidth = 10;
	windowWidth = 800;
	windowHeight = 600;
	windowX = 100;
	windowY = 100;
	StringCchPrintf(fileNameHeading, MAX_PATH, TEXT("Безымянный"));
	StringCchPrintf(fileName, 1, TEXT(""));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	handle = hInstance;
	if (!RegisterClassEx(&RegisterWindowClass(hInstance)))
		return EXIT_FAILURE;
	LoadLibrary(TEXT("ComCtl32.dll"));
	hFile = CreateFile(initializationFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		CloseHandle(hFile);
		StandardParameters();
	}
	else {
		CloseHandle(hFile);
		LoadInitializationFile();
	}
	hWnd = CreateWindowEx(0, TEXT("WindowClass"), TEXT(""), WS_OVERLAPPED | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
		return EXIT_FAILURE;
	ShowWindow(hWnd, nCmdShow);
	SetFocus(hTextEdit);
	MessagesProcessing();
	SaveInitializationFile();
	return EXIT_SUCCESS;
}