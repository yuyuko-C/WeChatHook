#pragma once
#include <string>
#include <vector>
#include <sstream>

class Tools
{
public:
	static DWORD  GetProcessIdbyName(std::wstring processName);
	static HANDLE GetProcessHandle(DWORD processId);
	static DWORD InjectedDllAddress(HANDLE hProcess, const std::wstring dllName);
	static LPVOID WriteTextToMemory(HANDLE hProcess, const std::wstring text);
	static DWORD InjectDll(HANDLE hProcess, std::wstring dllPath);
	static bool FreeDll(HANDLE hProcess, DWORD dllAddr);


	static std::wstring BrowserFileWindow(const WCHAR* lpstrFilter);
	static std::vector<std::wstring> split(const std::wstring& s, TCHAR delim);
	static void split(const std::wstring& s, TCHAR delim, std::vector<std::wstring>& elems);
	static std::wstring ToFullPath(std::wstring path);
	static void OpenExplorerAndSelect(std::wstring& path);
	static HWND FindDialog(std::wstring title);
	static void OpenConsole();
	static void CloseConsole();
	static bool GetBaseBoardByCmd(char* lpszBaseBoard, int len = 128);
	static bool isFileExists_fopen(std::wstring& filePath);


	static std::string CharStringToHexString(std::string);
	static std::string HexArrayToCharString(std::string);
	static std::wstring String2Wstring(std::string sToMatch);
	static std::string Wstring2String(std::wstring sToMatch);


	static int GetRandomInt(int from, int to);
	static bool CurrentDateEarlierThan(int year, int month, int day);
};

