#include<iostream>
#include<Windows.h>
#include<locale.h>
using namespace std;

VOID InstalledApplications() {
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
		DWORD keysCount, bufSize;
		if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, &keysCount, &bufSize, NULL, NULL, NULL, NULL, NULL, NULL)) {
			CHAR keyName[MAX_PATH];
			for (INT i = 0; i < keysCount; i++) {
				HKEY hKeyKey;
				DWORD bufSize2 = MAX_PATH;
				if (ERROR_SUCCESS == RegEnumKeyEx(hKey, i, keyName, &bufSize2, NULL, NULL, NULL, NULL)) {
					if (RegOpenKeyEx(hKey, keyName, REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKeyKey) == ERROR_SUCCESS) {
						DWORD type;
						if ((RegQueryValueEx(hKeyKey, TEXT("DisplayName"), NULL, &type, NULL, NULL) == ERROR_SUCCESS) && (type == REG_SZ)) {
							CHAR result[MAX_PATH];
							DWORD bufSize3 = MAX_PATH;
							if (ERROR_SUCCESS == RegQueryValueEx(hKeyKey, TEXT("DisplayName"), NULL, NULL, (LPBYTE)result, &bufSize3))
								cout << result << endl;
						}
					}
					RegCloseKey(hKeyKey);
				}
			}
			RegCloseKey(hKey);
		}
	}
}

VOID StartupApplications(){
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
		DWORD valuesCount;
		if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &valuesCount, NULL, NULL, NULL, NULL)) {
			for (INT i = 0; i < valuesCount; i++) {
				CHAR name[MAX_PATH], value[MAX_PATH];
				DWORD bufSize3 = MAX_PATH, type;
				if ((ERROR_SUCCESS == RegEnumValue(hKey, i, name, &bufSize3, NULL, &type, (LPBYTE)value, &bufSize3)) && ((REG_SZ == type) || (REG_EXPAND_SZ == type)))
					cout << value << endl;
			}
		}
		RegCloseKey(hKey);
	}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
		DWORD valuesCount;
		if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &valuesCount, NULL, NULL, NULL, NULL)) {
			for (INT i = 0; i < valuesCount; i++) {
				CHAR name[MAX_PATH], value[MAX_PATH];
				DWORD bufSize3 = MAX_PATH, type;
				if ((ERROR_SUCCESS == RegEnumValue(hKey, i, name, &bufSize3, NULL, &type, (LPBYTE)value, &bufSize3)) && ((REG_SZ == type) || (REG_EXPAND_SZ == type)))
					cout << value << endl;
			}
		}
		RegCloseKey(hKey);
	}
}

INT main() {
	setlocale(LC_ALL, "rus");
	cout << "Установленные программы" << endl << "------------------------------------------" << endl;
	InstalledApplications();
	cout<< endl<< "------------------------------------------"<< endl<<"Программы автозапуска" << endl << "------------------------------------------" << endl;
	StartupApplications();
	system("pause");
}
