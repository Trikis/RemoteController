#include <windows.h>
#include <string>
#include <iostream>
#include <wininet.h>
#include <shlobj_core.h>
#pragma comment(lib , "wininet.lib")
#pragma comment(lib , "urlmon.lib")

wchar_t url_Service[] = L"http://sameaist.beget.tech/Service.exe";
wchar_t url_API[] = L"http://sameaist.beget.tech/API.dll";
wchar_t url_MainClient[] = L"http://sameaist.beget.tech/MainClient.exe";
wchar_t url_RemoteAcces[] = L"http://sameaist.beget.tech/RemoteAccess.exe";
wchar_t url_ScreenModule[] = L"http://sameaist.beget.tech/ScreenModule.exe";

std::wstring ServiceFile;
std::wstring APIFile;
std::wstring MainClientFile;
std::string LogFile;
std::wstring RemoteAccesFile;
std::wstring ScreenModuleFile;

BOOL dirExists(const wchar_t* dir_path) {
	DWORD dwAttributes = GetFileAttributes(dir_path);
	if (dwAttributes == INVALID_FILE_ATTRIBUTES) return FALSE;
	if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) return TRUE;
	return FALSE;
}


int main(int argc, char** argv) {

	char AppData[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, AppData);
	LogFile = std::string(AppData) + "\\RemoteController\\RemoteController.log";

	wchar_t system_path[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, system_path) != S_OK) {
		std::cout << "SHGetFolderPath failed" << std::endl;
		return -1;
	}
	std::wstring check_path = system_path;
	check_path += L"\\RemoteController";

	if (dirExists(check_path.c_str())){
		std::cout << "You already installed Programm" << std::endl;
		return -1;
	}
	if (CreateDirectory(check_path.c_str(), NULL) == 0) {
		std::cout << "CreateDirectory failed" << std::endl;
		return -1;
	}    

	ServiceFile  = check_path +  L"\\Service.exe";
	APIFile = check_path + L"\\API.dll";
	MainClientFile = check_path + L"\\MainClient.exe";
	RemoteAccesFile = check_path + L"\\RemoteAccess.exe";
	ScreenModuleFile = check_path + L"\\ScreenModule.exe";

	DeleteUrlCacheEntryW(url_Service);
	DeleteUrlCacheEntryW(url_API);
	DeleteUrlCacheEntryW(url_MainClient);
	DeleteUrlCacheEntryW(url_RemoteAcces);
	DeleteUrlCacheEntryW(url_ScreenModule);

	if (URLDownloadToFileW(NULL, url_Service, ServiceFile.c_str(), 0, NULL) != S_OK) {
		std::cout << "Can not download Service.exe" << std::endl;
		return -1;
	}

	if (URLDownloadToFileW(NULL, url_API, APIFile.c_str(), 0, NULL) != S_OK) {
		std::cout << "Can not download API.dll" << std::endl;
		return -1;
	}
	if (URLDownloadToFileW(NULL, url_MainClient, MainClientFile.c_str(), 0, NULL) != S_OK) {
		std::cout << "Can not download MainClient.exe" << std::endl;
		return -1;
	}
	if (URLDownloadToFileW(NULL, url_RemoteAcces, RemoteAccesFile.c_str(), 0, NULL) != S_OK) {
		std::cout << "Can not download RemoteAccess.exe" << std::endl;
		return -1;
	}
	if (URLDownloadToFileW(NULL, url_ScreenModule, ScreenModuleFile.c_str(), 0, NULL) != S_OK) {
		std::cout << "Can not download ScreenModule.exe" << std::endl;
		return -1;
	}


	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (!CreateProcessW(ServiceFile.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		std::cout << "Create Process error" << std::endl;
		return -1;
	}


	std::cout << "Service files installed..." << std::endl;
	std::cout << "For information about service working , check service log file: " << LogFile << std::endl;


	std::cout << "Press any key to exit ...";
	char ch; std::cin >> ch;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}
