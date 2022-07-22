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

	// 注入模块名称
	LPVOID dllNameAddr = WriteTextToMemory(hProcess, dllName);
	if (dllNameAddr == NULL) {
		//MessageBox(NULL, L"模块名称写入失败,请重启微信后重试", L"错误", 0);
		return NULL;
	}

	// 获取到模块地址
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
		//MessageBox(NULL, L"内存分配失败,重启微信后重试", L"错误", 0);
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

	// 写入dll路径
	LPVOID dllPathAddr = Tools::WriteTextToMemory(hProcess, dllPath);
	if (dllPathAddr == NULL)
	{
		//MessageBox(NULL, L"DLL路径写入目标进程失败,重启微信后重试", L"错误", 0);
		return NULL;
	}

	// 获取loadLibraryW函数地址
	HMODULE kernel32 = GetModuleHandle(L"kernel32.dll");
	if (kernel32 == NULL) { MessageBox(NULL, L"未找kernel32模块", L"错误", 0);  return NULL; }
	LPVOID libraryAddr = GetProcAddress(kernel32, "LoadLibraryW");
	if (libraryAddr == NULL)
	{
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		//MessageBox(NULL, L"未找LoadLibraryW函数", L"错误", 0);
		return NULL;
	}
	// 远程命令载入DLL
	HANDLE exec = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)libraryAddr, dllPathAddr, 0, NULL);
	if (exec == NULL) {
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		//MessageBox(NULL, L"Dll注入失败", L"错误", 0);
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
		//MessageBox(NULL, L"未找kernel32模块", L"错误", 0); 
		return false;
	}
	LPVOID FreelibraryAddr = GetProcAddress(kernel32, "FreeLibrary");

	HANDLE exec1 = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)FreelibraryAddr, (LPVOID)dllAddr, 0, NULL);
	if (exec1 == NULL)
	{
		VirtualFreeEx(hProcess, (LPVOID)dllAddr, 0, MEM_RELEASE);
		//MessageBox(NULL, L"Dll释放失败", L"错误", 0);
		return false;
	}

	return true;
}

std::wstring Tools::BrowserFileWindow(const WCHAR* lpstrFilter)
{
	OPENFILENAME ofn;// 
	TCHAR szFile[MAX_PATH];// 保存获取?件名称的缓冲区
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = lpstrFilter; // L"All\0*.*\0Text\0*.TXT\0Image\0*.PNG;*.JPG\0";//过滤规则
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;

	ofn.lpstrInitialDir = L"C:\\Program Files";//指定默认路径
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
	//打开控制台,绑定到标准输入输出
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
	const long MAX_COMMAND_SIZE = 10000; // 命令行输出缓冲大小	
	WCHAR szFetCmd[] = L"wmic BaseBoard get SerialNumber"; // 获取主板序列号命令行	
	const std::string strEnSearch = "SerialNumber"; // 主板序列号的前导信息
	int j = 0;

	BOOL   bret = FALSE;
	HANDLE hReadPipe = NULL; //读取管道
	HANDLE hWritePipe = NULL; //写入管道	
	PROCESS_INFORMATION pi;   //进程信息	
	STARTUPINFO			si;	  //控制命令行窗口信息
	SECURITY_ATTRIBUTES sa;   //安全属性

	char			szBuffer[MAX_COMMAND_SIZE + 1] = { 0 }; // 放置命令行结果的输出缓冲区
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

	//1.0 创建管道
	bret = CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	if (!bret)
	{
		goto END;
		return false;
	}

	//2.0 设置命令行窗口的信息为指定的读写管道
	GetStartupInfo(&si);
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.wShowWindow = SW_HIDE; //隐藏命令行窗口
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	//3.0 创建获取命令行的进程
	bret = CreateProcess(NULL, szFetCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (!bret)
	{
		goto END;
		return false;
	}

	//4.0 读取返回的数据
	WaitForSingleObject(pi.hProcess, 500/*INFINITE*/);
	bret = ReadFile(hReadPipe, szBuffer, MAX_COMMAND_SIZE, &count, 0);
	if (!bret)
	{
		goto END;
		return false;
	}

	//5.0 查找主板序列号
	bret = FALSE;
	strBuffer = szBuffer;
	ipos = strBuffer.find(strEnSearch);

	if (ipos < 0) // 没有找到
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

	//去掉中间的空格 \r \n
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
	//关闭所有的句柄
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
	int iWLen = MultiByteToWideChar(CP_ACP, 0, sToMatch.c_str(), sToMatch.size(), 0, 0); // 计算转换后宽字符串的长度。（不包含字符串结束符）
	wchar_t* lpwsz = new wchar_t[iWLen + 1];
	MultiByteToWideChar(CP_ACP, 0, sToMatch.c_str(), sToMatch.size(), lpwsz, iWLen); // 正式转换。
	lpwsz[iWLen] = L'\0';
	std::wstring wsToMatch(lpwsz);
	delete[]lpwsz;
	return wsToMatch;
}

std::string Tools::Wstring2String(std::wstring sToMatch)
{
	std::string sResult;
	int iLen = WideCharToMultiByte(CP_ACP, NULL, sToMatch.c_str(), -1, NULL, 0, NULL, FALSE); // 计算转换后字符串的长度。（包含字符串结束符）
	char* lpsz = new char[iLen];
	WideCharToMultiByte(CP_OEMCP, NULL, sToMatch.c_str(), -1, lpsz, iLen, NULL, FALSE); // 正式转换。
	sResult.assign(lpsz, iLen - 1); // 对string对象进行赋值。
	delete[]lpsz;
	return sResult;
}

int Tools::GetRandomInt(int from, int to)
{
	//更新随机种子
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
