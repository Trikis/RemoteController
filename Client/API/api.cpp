#include "api.h"

//логирование
BOOL dirExists(const char* path) {
	DWORD dwAttributes = GetFileAttributesA(path);
	if (dwAttributes == INVALID_FILE_ATTRIBUTES) return FALSE;
	if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) return TRUE;
	return FALSE;
}

void Init() {
	char AppData[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, AppData);
	Log_File = AppData;
	Log_File += "\\RemoteController";
	if (!dirExists(Log_File.c_str())) {
		CreateDirectoryA(Log_File.c_str(), NULL);
	}
	Log_File += "\\RemoteController.log";
}

void LoadMessage(const char* message) {
	if (Log_File == "") Init();
	std::fstream out(Log_File, std::ios::out | std::ios::app);
	SYSTEMTIME st;
	GetSystemTime(&st);
	out << message << "\t----\t" << st.wHour << ":" << st.wMinute << ":" << st.wSecond
		<< " " << st.wDay << "." << st.wMonth << "." << st.wYear << std::endl;
	out.close();
}





//скриншот 

BOOL SaveScreen(const WCHAR* wPath) {
	BITMAPFILEHEADER bfHeader;
	BITMAPINFOHEADER biHeader;
	BITMAPINFO bInfo;
	HGDIOBJ hTempBitmap;
	HBITMAP hBitMap;
	BITMAP bAllDesktops;
	HDC hDC, hMemDC;
	DWORD dwWidth, dwHeight;
	BYTE* bBits = NULL;
	HANDLE hHeap = GetProcessHeap();
	DWORD cbBits, dwWritten = 0;
	HANDLE hFile;
	INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

	ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
	ZeroMemory(&bInfo, sizeof(BITMAPINFO));
	ZeroMemory(&bAllDesktops, sizeof(BITMAP));

	hDC = GetDC(NULL);
	hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
	GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

	dwWidth = bAllDesktops.bmWidth;
	dwHeight = bAllDesktops.bmHeight;

	DeleteObject(hTempBitmap);

	bfHeader.bfType = (WORD)('B' | ('M' << 8));
	bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	biHeader.biSize = sizeof(BITMAPINFOHEADER);
	biHeader.biBitCount = 24;
	biHeader.biCompression = BI_RGB;
	biHeader.biPlanes = 1;
	biHeader.biWidth = dwWidth;
	biHeader.biHeight = dwHeight;

	bInfo.bmiHeader = biHeader;

	cbBits = (((24 * dwWidth + 31) & ~31) / 8) * dwHeight;

	hMemDC = CreateCompatibleDC(hDC);
	hBitMap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
	SelectObject(hMemDC, hBitMap);
	BitBlt(hMemDC, 0, 0, dwWidth, dwHeight, hDC, x, y, SRCCOPY);

	hFile = CreateFileW(wPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
	WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);

	CloseHandle(hFile);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hDC);
	DeleteObject(hBitMap);
	return TRUE;
}




//воспроизведение звука

Mp3Player::Mp3Player() {
	this -> gb = NULL;
	this -> mc = NULL;
	this -> me = NULL;
	this -> ba = NULL;
	this -> ms = NULL;
	this -> ready = false;
}

Mp3Player::~Mp3Player() {
	this->Clean();
}

bool Mp3Player::Load(LPCWSTR FileName) {
	this -> Clean();
	ready = false;

	HRESULT CreateInstanceResult = CoCreateInstance(CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,
		(void**)&this->gb);

	if (SUCCEEDED(CreateInstanceResult))
	{
		gb->QueryInterface(IID_IMediaControl, (void**)&mc);
		gb->QueryInterface(IID_IMediaEventEx, (void**)&me);
		gb->QueryInterface(IID_IBasicAudio, (void**)&ba);
		gb->QueryInterface(IID_IMediaSeeking, (void**)&ms);

		HRESULT hr = gb->RenderFile(FileName, NULL);
		if (SUCCEEDED(hr))
		{
			ready = true;
			if (ms)
			{
				ms->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
			}
		}

	}

	return ready;
}

void Mp3Player::Clean() {
	if (mc)
		mc->Stop();

	if (gb)
	{
		gb->Release();
		gb = NULL;
	}

	if (mc)
	{
		mc->Release();
		mc = NULL;
	}

	if (me)
	{
		me->Release();
		me = NULL;
	}

	if (ba)
	{
		ba->Release();
		ba = NULL;
	}

	if (ms)
	{
		ms->Release();
		ms = NULL;
	}
	ready = false;
}

bool Mp3Player::Play() {
	if (ready && mc)
	{
		HRESULT hr = mc->Run();
		return SUCCEEDED(hr);
	}
	return false;
}

bool Mp3Player::Pause() {
	if (ready && mc)
	{
		HRESULT hr = mc->Pause();
		return SUCCEEDED(hr);
	}  
	return false;
}

bool Mp3Player::Stop() {
	if (ready && mc)
	{
		HRESULT hr = mc->Stop();
		return SUCCEEDED(hr);
	}
	return false;
}

bool Mp3Player::WaitForEnding(long msTimeout, long* Code) {
	if (me && ready)
	{
		HRESULT hr = me->WaitForCompletion(msTimeout, Code);
		return *Code > 0;
	}

	return false;
}

BOOL ChangeVolumeToMax() {
	HRESULT hr;
	IMMDeviceEnumerator* deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
	if (FAILED(hr)) return FALSE;

	IMMDevice* defaultDevice = NULL;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	if (FAILED(hr)) return FALSE;

	IAudioEndpointVolume* endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER,
		NULL, (LPVOID*)&endpointVolume);

	defaultDevice->Release();
	if (FAILED(hr)) return FALSE;

	hr = endpointVolume->SetMasterVolumeLevelScalar(1.0, NULL);
	endpointVolume->Release();

	return SUCCEEDED(hr);
}

DWORD WINAPI PlaySomeSound(LPVOID FilePath) {
	WaitForSingleObject(hEvent, INFINITE);

	CoInitialize(NULL);
	ChangeVolumeToMax();
	Mp3Player mp3;
	mp3.Load((wchar_t*)FilePath);
	mp3.Play();
	long evCode;
	while (mp3.WaitForEnding(0, &evCode) != TRUE) {

	}
	DeleteFile((wchar_t*)FilePath);
	CoUninitialize();

	SetEvent(hEvent);
	return ERROR_SUCCESS;
}

//Shutdown

VOID ShutDownThisSystem() {
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken)) {
		return;
	}

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

	if (GetLastError() != ERROR_SUCCESS) {
		return;
	}

	if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_FLAG_PLANNED)) {
		return;
	}
}

void AddToRegisteryAutorunScreenModule() {
	wchar_t szPath[] = L"C:\\Program Files\\RemoteController\\ScreenModule.exe";
	HKEY hKey;
	RegOpenKeyExW(
		HKEY_LOCAL_MACHINE,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		KEY_SET_VALUE | KEY_READ,
		&hKey
	);

	WCHAR szBuffer[MAX_PATH];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG lError = RegQueryValueExW(hKey, L"ScreenModule", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (lError != ERROR_SUCCESS) {
		RegSetValueExW(hKey, L"ScreenModule", NULL, REG_SZ, (LPBYTE)szPath, sizeof(szPath));
	}
}

void AddToRegisteryAutorunMainClient() {
	wchar_t szPath[] = L"C:\\Program Files\\RemoteController\\MainClient.exe";
	HKEY hKey;
	RegOpenKeyExW(
		HKEY_LOCAL_MACHINE,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		KEY_SET_VALUE | KEY_READ,
		&hKey
	);

	WCHAR szBuffer[MAX_PATH];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG lError = RegQueryValueExW(hKey, L"MainClient", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (lError != ERROR_SUCCESS) {
		RegSetValueExW(hKey, L"MainClient", NULL, REG_SZ, (LPBYTE)szPath, sizeof(szPath));
	}
}

void CheckAutoRun() {
	AddToRegisteryAutorunScreenModule();
	AddToRegisteryAutorunMainClient();
}