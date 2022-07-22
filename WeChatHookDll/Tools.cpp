#include "pch.h"
#include "Tools.h"
#include <Commdlg.h>
#include <TlHelp32.h>
#include <WinUser.h>
#include <iostream>
#include <algorithm>





DWORD Tools::GetProcessIdbyName(std::wstring processName)
{
	if (processName.empty())
	{
		return NULL;
	}

	HANDLE shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 info = { 0 };
	info.dwSize = sizeof(PROCESSENTRY32);
	do
	{
		if (wcscmp(processName.c_str(), info.szExeFile) == 0)
		{
			return info.th32ProcessID;
		}
	} while (Process32Next(shot, &info));
	return NULL;
}

HANDLE Tools::GetProcessHandle(DWORD processId)
{
	if (processId == 0)
	{
		return NULL;
	}
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (hProcess == NULL)
	{
		return NULL;
	}
	else
	{
		return hProcess;
	}
}

DWORD Tools::InjectedDllAddress(HANDLE hProcess, const std::wstring dllName)
{
	if (hProcess == NULL || dllName.empty())
	{
		return NULL;
	}

	// ע��ģ������
	LPVOID dllNameAddr = WriteTextToMemory(hProcess, dllName);
	if (dllNameAddr == NULL) {
		//MessageBox(NULL, L"ģ������д��ʧ��,������΢�ź�����", L"����", 0);
		return NULL;
	}

	// ��ȡ��ģ���ַ
	HANDLE exec = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetModuleHandle, dllNameAddr, 0, NULL);
	if (exec == NULL)
	{
		VirtualFreeEx(hProcess, dllNameAddr, 0, MEM_RELEASE);
		return NULL;
	}
	else
	{
		DWORD h = NULL;
		WaitForSingleObject(exec, INFINITE);
		GetExitCodeThread(exec, &h);
		if (h == NULL)
		{
			VirtualFreeEx(hProcess, dllNameAddr, 0, MEM_RELEASE);
			return NULL;
		}
		else
		{
			return h;
		}
	}
}

LPVOID Tools::WriteTextToMemory(HANDLE hProcess, const std::wstring text)
{
	if (hProcess == NULL || text.empty())
	{
		return NULL;
	}

	LPVOID dllPathAddr = VirtualAllocEx(hProcess, NULL, wcslen(text.c_str()) * 2, MEM_COMMIT, PAGE_READWRITE);
	if (dllPathAddr == NULL)
	{
		//MessageBox(NULL, L"�ڴ����ʧ��,����΢�ź�����", L"����", 0);
		return NULL;
	}
	BOOL success = WriteProcessMemory(hProcess, dllPathAddr, text.c_str(), wcslen(text.c_str()) * 2, NULL);
	if (!success)
	{
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		return NULL;
	}

	return dllPathAddr;
}

DWORD Tools::InjectDll(HANDLE hProcess, std::wstring dllPath)
{
	if (hProcess == NULL || dllPath.empty())
	{
		return NULL;
	}

	// д��dll·��
	LPVOID dllPathAddr = Tools::WriteTextToMemory(hProcess, dllPath);
	if (dllPathAddr == NULL)
	{
		//MessageBox(NULL, L"DLL·��д��Ŀ�����ʧ��,����΢�ź�����", L"����", 0);
		return NULL;
	}

	// ��ȡloadLibraryW������ַ
	HMODULE kernel32 = GetModuleHandle(L"kernel32.dll");
	if (kernel32 == NULL) { MessageBox(NULL, L"δ��kernel32ģ��", L"����", 0);  return NULL; }
	LPVOID libraryAddr = GetProcAddress(kernel32, "LoadLibraryW");
	if (libraryAddr == NULL)
	{
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		//MessageBox(NULL, L"δ��LoadLibraryW����", L"����", 0);
		return NULL;
	}
	// Զ����������DLL
	HANDLE exec = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)libraryAddr, dllPathAddr, 0, NULL);
	if (exec == NULL) {
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		//MessageBox(NULL, L"Dllע��ʧ��", L"����", 0);
		return NULL;
	}
	WaitForSingleObject(exec, INFINITE);
	DWORD dllAddr;
	GetExitCodeThread(exec, &dllAddr);
	return dllAddr;

}

bool Tools::FreeDll(HANDLE hProcess, DWORD dllAddr)
{
	if (hProcess == NULL || dllAddr == NULL)
	{
		return false;
	}

	HMODULE kernel32 = GetModuleHandle(L"kernel32.dll");
	if (kernel32 == NULL) {
		//MessageBox(NULL, L"δ��kernel32ģ��", L"����", 0); 
		return false;
	}
	LPVOID FreelibraryAddr = GetProcAddress(kernel32, "FreeLibrary");

	HANDLE exec1 = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)FreelibraryAddr, (LPVOID)dllAddr, 0, NULL);
	if (exec1 == NULL)
	{
		VirtualFreeEx(hProcess, (LPVOID)dllAddr, 0, MEM_RELEASE);
		//MessageBox(NULL, L"Dll�ͷ�ʧ��", L"����", 0);
		return false;
	}

	return true;
}

std::wstring Tools::BrowserFileWindow(const WCHAR* lpstrFilter)
{
	OPENFILENAME ofn;// 
	TCHAR szFile[MAX_PATH];// �����ȡ?�����ƵĻ�����
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = lpstrFilter; // L"All\0*.*\0Text\0*.TXT\0Image\0*.PNG;*.JPG\0";//���˹���
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;

	ofn.lpstrInitialDir = L"C:\\Program Files";//ָ��Ĭ��·��
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		return ofn.lpstrFile;
	}
	else
	{
		return L"";
	}
}

std::vector<std::wstring> Tools::split(const std::wstring& s, TCHAR delim)
{
	std::vector<std::wstring> elems;
	split(s, delim, elems);
	return elems;
}

void Tools::split(const std::wstring& s, TCHAR delim, std::vector<std::wstring>& elems)
{
	std::wstringstream ss;
	ss.str(s);
	std::wstring item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

std::wstring Tools::ToFullPath(std::wstring path)
{
	WCHAR buff[1024] = { 0 };
	std::wstring fullPath;
	if (_wfullpath((WCHAR*)buff, path.c_str(), 1024) != NULL)
	{
		fullPath = std::wstring(buff);
	}
	return fullPath;
}

void Tools::OpenExplorerAndSelect(std::wstring& path)
{
	std::wstring fullPath = ToFullPath(path);
	WCHAR buffer[0x100] = { 0 };
	wsprintf(buffer, L"explorer /select, %s", fullPath.c_str());
	_wsystem(buffer);
}

HWND Tools::FindDialog(std::wstring title)
{
	return FindWindow(L"#32770", title.c_str());
}

void Tools::OpenConsole()
{
	//�򿪿���̨,�󶨵���׼�������
	AllocConsole();
	FILE* ofp, * ifp;
	freopen_s(&ofp, "CONOUT$", "w", stdout);
	freopen_s(&ifp, "CONIN$", "r", stdin);
}

void Tools::CloseConsole()
{
	FreeConsole();
}

bool Tools::GetBaseBoardByCmd(char* lpszBaseBoard, int len)
{
	const long MAX_COMMAND_SIZE = 10000; // ��������������С	
	WCHAR szFetCmd[] = L"wmic BaseBoard get SerialNumber"; // ��ȡ�������к�������	
	const std::string strEnSearch = "SerialNumber"; // �������кŵ�ǰ����Ϣ
	int j = 0;

	BOOL   bret = FALSE;
	HANDLE hReadPipe = NULL; //��ȡ�ܵ�
	HANDLE hWritePipe = NULL; //д��ܵ�	
	PROCESS_INFORMATION pi;   //������Ϣ	
	STARTUPINFO			si;	  //���������д�����Ϣ
	SECURITY_ATTRIBUTES sa;   //��ȫ����

	char			szBuffer[MAX_COMMAND_SIZE + 1] = { 0 }; // ���������н�������������
	std::string			strBuffer;
	unsigned long	count = 0;
	long			ipos = 0;

	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	memset(&sa, 0, sizeof(sa));

	pi.hProcess = NULL;
	pi.hThread = NULL;
	si.cb = sizeof(STARTUPINFO);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//1.0 �����ܵ�
	bret = CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	if (!bret)
	{
		goto END;
		return false;
	}

	//2.0 ���������д��ڵ���ϢΪָ���Ķ�д�ܵ�
	GetStartupInfo(&si);
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.wShowWindow = SW_HIDE; //���������д���
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	//3.0 ������ȡ�����еĽ���
	bret = CreateProcess(NULL, szFetCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (!bret)
	{
		goto END;
		return false;
	}

	//4.0 ��ȡ���ص�����
	WaitForSingleObject(pi.hProcess, 500/*INFINITE*/);
	bret = ReadFile(hReadPipe, szBuffer, MAX_COMMAND_SIZE, &count, 0);
	if (!bret)
	{
		goto END;
		return false;
	}

	//5.0 �����������к�
	bret = FALSE;
	strBuffer = szBuffer;
	ipos = strBuffer.find(strEnSearch);

	if (ipos < 0) // û���ҵ�
	{
		goto END;
		return false;
	}
	else
	{
		strBuffer = strBuffer.substr(ipos + strEnSearch.length());
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	strcpy_s(szBuffer, strBuffer.c_str());

	//ȥ���м�Ŀո� \r \n
	//int j = 0;
	for (int i = 0; i < strlen(szBuffer); i++)
	{
		if (szBuffer[i] != ' ' && szBuffer[i] != '\n' && szBuffer[i] != '\r')
		{
			lpszBaseBoard[j] = szBuffer[i];
			j++;
		}
	}

	bret = TRUE;

END:
	//�ر����еľ��
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return(bret);
}

bool Tools::isFileExists_fopen(std::wstring& filePath)
{
	FILE* file;
	errno_t err = _wfopen_s(&file, filePath.c_str(), L"r");

	if (err == 0) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

std::string Tools::CharStringToHexString(std::string src)
{
	int chLength = src.length();
	std::string ret;
	static const char* const lut = "0123456789ABCDEF";
	size_t len = src.length();

	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = src[i];
		ret += lut[c >> 4];
		ret += lut[c & 15];
	}
	std::transform(ret.begin(), ret.end(), ret.begin(), std::toupper);
	return ret;
}

std::string Tools::HexArrayToCharString(std::string src)
{
	int srcLength = src.length();
	std::string ret;
	if (srcLength % 2 != 0)
	{
		throw "Invalid String Length";
	}
	else
	{
		for (size_t i = 0; i < srcLength; i += 2)
		{
			ret += (char)std::stoul(src.substr(i, 2), 0, 16);
		}
	}
	return ret;
}

std::wstring Tools::String2Wstring(std::string sToMatch)
{
	int iWLen = MultiByteToWideChar(CP_ACP, 0, sToMatch.c_str(), sToMatch.size(), 0, 0); // ����ת������ַ����ĳ��ȡ����������ַ�����������
	wchar_t* lpwsz = new wchar_t[iWLen + 1];
	MultiByteToWideChar(CP_ACP, 0, sToMatch.c_str(), sToMatch.size(), lpwsz, iWLen); // ��ʽת����
	lpwsz[iWLen] = L'\0';
	std::wstring wsToMatch(lpwsz);
	delete[]lpwsz;
	return wsToMatch;
}

std::string Tools::Wstring2String(std::wstring sToMatch)
{
	std::string sResult;
	int iLen = WideCharToMultiByte(CP_ACP, NULL, sToMatch.c_str(), -1, NULL, 0, NULL, FALSE); // ����ת�����ַ����ĳ��ȡ��������ַ�����������
	char* lpsz = new char[iLen];
	WideCharToMultiByte(CP_OEMCP, NULL, sToMatch.c_str(), -1, lpsz, iLen, NULL, FALSE); // ��ʽת����
	sResult.assign(lpsz, iLen - 1); // ��string������и�ֵ��
	delete[]lpsz;
	return sResult;
}

int Tools::GetRandomInt(int from, int to)
{
	//�����������
	srand((unsigned)time(NULL));
	return (rand() % (to - from + 1)) + from;
}

bool Tools::CurrentDateEarlierThan(int year, int month, int day)
{
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);

	if (ltm.tm_year + 1900 < year)
	{
		return true;
	}
	else if (ltm.tm_year + 1900 > year)
	{
		return false;
	}
	else
	{
		if (ltm.tm_mon + 1 < month)
		{
			return true;
		}
		else if (ltm.tm_mon + 1 > month)
		{
			return false;
		}
		else
		{
			if (ltm.tm_mday < day)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}
