#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <shlobj_core.h>
#include <string>
#include "api.h"
#include <fstream>
#include <tlhelp32.h>
#include <WtsApi32.h>

#pragma comment(lib , "API.lib")
#pragma comment(lib , "Wtsapi32.lib")
#pragma warning(disable:4996)


wchar_t SERVICE_NAME[] = L"RemoteController";
bool isCreate = FALSE;


SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatus; //StatusHandle
HANDLE ServiceStopEvent = INVALID_HANDLE_VALUE;


VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode);
DWORD WINAPI ServiceWorkerThreadEntry(LPVOID lpParam);

VOID Install(VOID);
BOOL dirExists(const wchar_t* dir_path);


int main(int argc, char* argv[]) {
	FreeConsole();
	Install();

	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{SERVICE_NAME , (LPSERVICE_MAIN_FUNCTION)ServiceMain} ,
		{NULL , NULL}
	};
	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
		LoadMessage("Service.exe : StartServiceCtrlDIspathcer error");
		return GetLastError();
	}
	return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
	DWORD Status = E_FAIL;
	hServiceStatus = RegisterServiceCtrlHandler(
		SERVICE_NAME,
		ServiceCtrlHandler
	);
	if (hServiceStatus == NULL) {
		LoadMessage("Service.exe : RegisterServiceCtrlHandler failed");
		return;
	}
	ZeroMemory(&ServiceStatus, sizeof(ServiceStatus));
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
	ServiceStatus.dwControlsAccepted = 0;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(hServiceStatus, &ServiceStatus) == FALSE) {
		LoadMessage("Service.exe : SetServiceStatus failed");
	}

	ServiceStopEvent = CreateEventW(NULL, TRUE, FALSE, L"RemoteControllerEvent");
	if (ServiceStopEvent == NULL) {
		ServiceStatus.dwControlsAccepted = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = GetLastError();
		ServiceStatus.dwCheckPoint = 1;
		if (SetServiceStatus(hServiceStatus, &ServiceStatus) == FALSE) {
			LoadMessage("Service.exe : SetServiceStatus failed");
			return;
		}
	}

	//start
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(hServiceStatus, &ServiceStatus) == FALSE) {
		LoadMessage("Service.exe : SetServiceStatus failed");
	}
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThreadEntry, NULL, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(ServiceStopEvent);
	ServiceStatus.dwControlsAccepted = 0;
	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCheckPoint = 3;

	if (!SetServiceStatus(hServiceStatus, &ServiceStatus) == FALSE) {
		LoadMessage("Service.exe : SetServiceStatus failed");
	}
	return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
	switch (CtrlCode) {
	case SERVICE_CONTROL_STOP:
		if (ServiceStatus.dwCurrentState != SERVICE_RUNNING) break;

		ServiceStatus.dwControlsAccepted = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCheckPoint = 4;
		if (SetServiceStatus(hServiceStatus, &ServiceStatus) == FALSE) {
			LoadMessage("Service.exe : SetServiceStatus failed");
			return;
		}
		SetEvent(ServiceStopEvent);
	default:
		break;
	}
}


BOOL dirExists(const wchar_t* dir_path) {
	DWORD dwAttributes = GetFileAttributes(dir_path);
	if (dwAttributes == INVALID_FILE_ATTRIBUTES) return FALSE;
	if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) return TRUE;
	return FALSE;
}


VOID Install(VOID) {

	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	wchar_t curr_path[MAX_PATH];
	if (!GetModuleFileName(NULL, curr_path, MAX_PATH)) {
		LoadMessage("Service.exe : GetModuleFilename failed");
		return;
	}
	schSCManager = OpenSCManager(
		NULL, NULL, SC_MANAGER_ALL_ACCESS
	);
	if (schSCManager == NULL) {
		LoadMessage("Service.exe : OpenSCManager failed[with SC_MANAGER_ALL_ACCESS]");
		return;
	}
	schService = CreateService(
		schSCManager, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		curr_path, NULL, NULL, NULL, NULL, NULL);
	if (schService == NULL) {
		LoadMessage("Service.exe : CreateService failed"); 
		CloseServiceHandle(schSCManager);
		return;
	}

	if (StartService(schService , 0 , NULL) == 0){
		LoadMessage("Service.exe : Start service failed");
		CloseServiceHandle(schSCManager);
		CloseServiceHandle(schService);
		return;
	}
	
	LoadMessage("\tService instaled");
	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);
}

DWORD WINAPI ServiceWorkerThreadEntry(LPVOID lpParam) {
	CheckAutoRun();
	return ERROR_SUCCESS;
}




