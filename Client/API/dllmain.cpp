#include <windows.h>

BOOL WINAPI dllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpreserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}