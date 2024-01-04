#include<Windows.h>
#include <strsafe.h>
#include <Shlobj_core.h>

#define CHOOSE_FROM "Выберите файл или каталог для копирования..."
#define CHOOSE_TO "Выберите место назначения..."
#define FROM_FILE 1
#define FROM_CATALOG 2
#define TO_CATALOG 3
#define LETS_GO 4

HINSTANCE handle;
HWND hWnd = NULL, hCopyWhat, hCopyTo;
WIN32_FILE_ATTRIBUTE_DATA win32_file_attribute_data;
CHAR copyTo[MAX_PATH], copyWhat[MAX_PATH], fileShortName[MAX_PATH];

VOID CreatingElements(HINSTANCE hInstance, HWND hWnd)
{
	CreateWindow("BUTTON", "Что копируем", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 10, 600, 110, hWnd, (HMENU)1, hInstance, NULL);
	hCopyWhat = CreateWindow("static", CHOOSE_FROM, WS_CHILD | WS_VISIBLE, 20, 40, 580, 20, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("BUTTON", "Файл", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 70 , 150, 30, hWnd, (HMENU)FROM_FILE, hInstance, NULL); 
	CreateWindow("BUTTON", "Каталог", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 300, 70, 150, 30, hWnd, (HMENU)FROM_CATALOG, hInstance, NULL); 
	CreateWindow("BUTTON", "Куда копируем", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 130, 600, 110, hWnd, (HMENU)1, hInstance, NULL);
	hCopyTo = CreateWindow("static", CHOOSE_FROM, WS_CHILD | WS_VISIBLE, 20, 170, 580, 20, hWnd, (HMENU)1, hInstance, NULL);
	CreateWindow("BUTTON", "Место назначения", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 230, 200, 150, 30, hWnd, (HMENU)TO_CATALOG, hInstance, NULL);
	CreateWindow("BUTTON", "Поехали!", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 230, 260, 150, 50, hWnd, (HMENU)LETS_GO, hInstance, NULL);
}

VOID GetFileShortName() {
	INT i;
	for (i = MAX_PATH - 1; i >= 0; i--) {
		if (copyWhat[i] == '\\') {
			break;
		}
	}
	INT j = 0;
	while (copyWhat[i] != '\0') {
		fileShortName[j] = copyWhat[i];
		j++;
		i++;
	}
}

VOID GetNewFileName() {
	INT i;
	for (i = 0; i <MAX_PATH; i++) {
		if (copyTo[i] == '\0') {
			break;
		}
	}
	INT j = 0;
	while (fileShortName[j] != '\0') {
		copyTo[i] = fileShortName[j];
		i++;
		j++;
	}
}

VOID ClearingNames() {
	for (INT i = 0; i < MAX_PATH; i++) {
		copyWhat[i] = '\0';
		copyTo[i] = '\0';
		fileShortName[i] = '\0';
	}
	SendMessage(hCopyWhat, WM_SETTEXT, 0, (LPARAM)CHOOSE_FROM);
	SendMessage(hCopyTo, WM_SETTEXT, 0, (LPARAM)CHOOSE_TO);
}

VOID CopyCatalog(LPCSTR copywhat, BOOL firstTime, BOOL main, LPCSTR copyto) {
	INT i;
	CHAR CopyTo[MAX_PATH];
	for (INT i = 0; i< MAX_PATH; i++)
		CopyTo[i] = copyTo[i];
	for (i = 0; i < MAX_PATH; i++) {
		if (copyTo[i] == '\0') {
			break;
		}
	}
	INT k;
	for (k = MAX_PATH; k >= 0; k--) {
		if (copywhat[k] == '\\') {
			break;
		}
	}
	while (copywhat[k] != '\0') {  
		CopyTo[i] = copywhat[k];
		k++, i++;
	}
	if (firstTime && (CreateDirectoryEx(copywhat, CopyTo, NULL) == FALSE)) {
		MessageBox(hWnd, TEXT("Невозможно скопировать в данный каталог!"), "Ошибка", MB_OK | MB_ICONERROR);
		ClearingNames();
	}
	else {
		CHAR searchString[MAX_PATH];
		WIN32_FIND_DATA win32_find_data;
		StringCchPrintf(searchString, MAX_PATH, TEXT("%s%s"), copywhat, "\\*");
		HANDLE hFirstFile = FindFirstFile(searchString, &win32_find_data);
		if (hFirstFile != INVALID_HANDLE_VALUE) {
			FindNextFile(hFirstFile, &win32_find_data);
			while (FindNextFile(hFirstFile, &win32_find_data) !=FALSE) {
					if (win32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						CHAR name[MAX_PATH], name2[MAX_PATH];
						for (INT i = 0; i < MAX_PATH; i++) {
							name[i] = '\0';
							name2[i] = '\0';
						}
						INT r = 0, a=0;
						while (copywhat[r] != '\0') {
							name[a] = copywhat[r];
							a++, r++;
						}
						name[a] = '\\';
						a++;
						r = 0;
						while (win32_find_data.cFileName[r] != '\0') {
							name[a] = win32_find_data.cFileName[r];
							a++, r++;
						}
						r = 0, a = 0;
						while (copyto[r] != '\0') {
							name2[a] = copyto[r];
							a++, r++;
						}
						INT ind;
						for (INT g = MAX_PATH; g >= 0; g--) {
							if (copywhat[g] == '\\') {
								ind = g;
								break;
							}
						}
						ind++;
						name2[a] = '\\';
						a++;
						while (copywhat[ind] != '\0') {
							name2[a] = copywhat[ind];
							a++, ind++;
						}
						name2[a] = '\\';
						a++;
						r = 0;
						while (win32_find_data.cFileName[r] != '\0') {
							name2[a] = win32_find_data.cFileName[r];
							a++, r++;
						}
						CreateDirectory(name2, NULL);
						INT p;
						for (INT i = MAX_PATH; i >= 0; i--) {
							if (name2[i] == '\\') {
								p = i; 
								break;
							}
						}
						for (INT i = p; p < MAX_PATH; p++)
							name2[p] = '\0';
						CopyCatalog(name, FALSE, FALSE, name2);
					}
					else {
						CHAR FileName[MAX_PATH], FileName2[MAX_PATH];
						for (INT i = 0; i < MAX_PATH; i++) {
							FileName[i] = '\0';
							FileName2[i] = '\0';
						}
						INT r = 0, a = 0;
						while (copywhat[r] != '\0') {
							FileName[a] = copywhat[r];
							a++, r++;
						}
						FileName[a] = '\\';
						a++;
						r = 0;
						while (win32_find_data.cFileName[r] != '\0') {
							FileName[a] = win32_find_data.cFileName[r];
							a++, r++;
						}
						r = 0, a = 0;
						while (copyto[r] != '\0') {
							FileName2[a] = copyto[r];
							a++, r++;
						}
						FileName2[a] = '\\';
						a++;
						INT ind;
						for (INT g = MAX_PATH; g >= 0; g--) {
							if (copywhat[g] == '\\') {
								ind = g;
								break;
							}
						}
						ind++;
						while (copywhat[ind] != '\0') {
							FileName2[a] = copywhat[ind];
							a++, ind++;
						}
						FileName2[a] = '\\';
						a++;
						r = 0;
						while (win32_find_data.cFileName[r] != '\0') {
							FileName2[a] = win32_find_data.cFileName[r];
							a++, r++;
						}
						CopyFile(FileName, FileName2, TRUE);
					}
			}
		}
		if (main) {
			MessageBox(hWnd, TEXT("Копирование успешно завершено!"), "Успех", MB_OK | MB_ICONINFORMATION);
			ClearingNames();
		}
	}

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
		return 0;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == LETS_GO) {
			if ((copyTo[0] == '\0') || (copyWhat[0] == '\0')) {
				MessageBox(hWnd, TEXT("Ошибка. Выбраны не все файлы!"), NULL, MB_OK | MB_ICONERROR);
			}
			else {
				if (GetFileAttributesEx(copyWhat, GetFileExInfoStandard, &win32_file_attribute_data) != FALSE) {
					if (win32_file_attribute_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						INT i;
						for (i = 0; i < MAX_PATH; i++) {
							if (copyWhat[i] == '\0')
								break;
						}
						BOOL check = TRUE;
						for (INT j = 0, k=0; j < i; j++, k++) {
							if (copyTo[j] != copyWhat[k]){
								check = FALSE;
								break;
							}
						}
						if (check) {
							MessageBox(hWnd, TEXT("Невозможно скопировать в данный каталог!"), "Ошибка", MB_OK | MB_ICONERROR);
							ClearingNames();
						}
						else {
							CopyCatalog(copyWhat, TRUE, TRUE, copyTo);
						}
					}
					else {
						GetFileShortName();
						GetNewFileName();
						if (CopyFile(copyWhat, copyTo, TRUE) == FALSE) {
							MessageBox(hWnd, TEXT("Файл с таким именем уже существует или копирование в данную директорию запрещено!"), "Ошибка", MB_OK | MB_ICONERROR);
							ClearingNames();
						}
						else {
							MessageBox(hWnd, TEXT("Копирование успешно завершено!"), "Успех", MB_OK | MB_ICONINFORMATION);
							ClearingNames();
						}
					}
				}
				else {
					MessageBox(hWnd, TEXT("Копирование невозможно!"), "Ошибка", MB_OK | MB_ICONERROR);
					ClearingNames();
				}
			}
		}
		if (LOWORD(wParam) == FROM_FILE) {
			OPENFILENAME ofn = { sizeof(OPENFILENAME) };
			ofn.hInstance = handle;
			ofn.lpstrFilter = TEXT("Все файлы");
			ofn.lpstrFile = copyWhat;
			ofn.nMaxFile = _countof(copyWhat);
			ofn.lpstrTitle = TEXT("Выбрать файл");
			ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn) != FALSE) {
				SendMessage(hCopyWhat, WM_SETTEXT, 0, (LPARAM)copyWhat);
			}
		}
		if (LOWORD(wParam) == FROM_CATALOG) {
			BROWSEINFO bi = { hWnd, NULL, NULL, "Выбрать папку", BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_BROWSEFORCOMPUTER, NULL, NULL, 0 };
			LPCITEMIDLIST lpItemDList = SHBrowseForFolder(&bi);
			if (lpItemDList != NULL) {
				SHGetPathFromIDList(lpItemDList, (LPSTR)copyWhat);
				if (copyWhat[0] == '\0') {
					MessageBox(hWnd, TEXT("Ошибка. Невозможно выборать данный каталог!"), NULL, MB_OK | MB_ICONERROR);
				}
				else {
					SendMessage(hCopyWhat, WM_SETTEXT, 0, (LPARAM)copyWhat);
				}
			}
		}
		if (LOWORD(wParam) == TO_CATALOG) {
			BROWSEINFO bi = { hWnd, NULL, NULL, "Выбрать папку", BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_BROWSEFORCOMPUTER, NULL, NULL, 0 };
			LPCITEMIDLIST lpItemDList = SHBrowseForFolder(&bi);
			if (lpItemDList != NULL) {
				SHGetPathFromIDList(lpItemDList, (LPSTR)copyTo);
				if (copyTo[0] == '\0') {
					MessageBox(hWnd, TEXT("Ошибка. Невозможно выборать данный каталог!"), NULL, MB_OK | MB_ICONERROR);
				}
				else {
					SendMessage(hCopyTo, WM_SETTEXT, 0, (LPARAM)copyTo);
				}
			}
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	handle = hInstance;
	if (!RegisterClassEx(&RegisterWindowClass(hInstance)))
		return EXIT_FAILURE;
	LoadLibrary(TEXT("ComCtl32.dll"));
	hWnd = CreateWindowEx(0, TEXT("WindowClass"), TEXT("Копирование"), WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, 100, 100, 635, 360, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
		return EXIT_FAILURE;
	ShowWindow(hWnd, nCmdShow);
	MessagesProcessing();
	return EXIT_SUCCESS;
}