#include "framework.h"
#include "resource.h"
#include "Tools.h"
#include "HookClient.h"
#include "RSA.h"
#include <shlobj.h>
#include <Windows.h>
#include <iostream>
#include <cstdlib>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define PROCESSNAME L"WeChat.exe"
#define DLLNAME L"WeChatHookDll.dll"

INT_PTR InjectDlgproc(HWND handle, UINT msg, WPARAM button, LPARAM unnamedParam4);
INT_PTR SocketDlgproc(HWND handle, UINT msg, WPARAM button, LPARAM unnamedParam4);
RSA GetRSAInstance();// 初始化rsa
std::string GetBoardId();// 获取电脑主板ID的十六进制写法
std::string GetCheckFileInfo(std::wstring& filePath);// 获取验证文件字符串


std::wstring exeFolder;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	exeFolder = Tools::ToFullPath(L"./");
	RSA rsa = GetRSAInstance();

#ifndef NDEBUG
	Tools::OpenConsole();
	std::cout << "输入唯一ID:";
	std::string in;
	std::cin >> in;
	std::cout << rsa.encryptByPublic(in) << std::endl;
	system("pause");
#else
	//Tools::OpenConsole();
	//执行身份验证
	std::wstring path = L"./6EB4A12509EC32BCF03D3C359315B282.db";
	std::string hexboard = GetBoardId();
	std::string content = GetCheckFileInfo(path);
	if (content.empty())
	{
		MessageBox(NULL, Tools::String2Wstring(hexboard).c_str(), L"", 0);
	}
	else
	{
		//验证通过
		if (rsa.decryptByPrivate(content) == hexboard)
		{
			DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)InjectDlgproc);
		}
		else
		{
			MessageBox(NULL, Tools::String2Wstring(hexboard).c_str(), L"", 0);
		}
	}
#endif // NDEBUG



	return 0;
}

DWORD processId = NULL;
HANDLE hProcess = NULL;
DWORD dllAddress = NULL;
HookClient client;
HWND consoleHandle = NULL;

INT_PTR InjectDlgproc(HWND handle, UINT msg, WPARAM button, LPARAM unnamedParam4)
{
	if (msg == WM_INITDIALOG)
	{
		client.StartUp();
	}
	else if (msg == WM_CLOSE)
	{
		EndDialog(handle, 0);
	}
	else if (msg == WM_COMMAND)
	{
		processId = Tools::GetProcessIdbyName(PROCESSNAME);
		if (processId == NULL)
		{
			MessageBox(NULL, L"微信已退出,请重启微信后重试", L"错误", 0);
			return 0;
		}
		else
		{
			hProcess = Tools::GetProcessHandle(processId);
			if (hProcess == NULL)
			{
				MessageBox(NULL, L"打开进程失败,重启微信后重试", L"错误", 0);
				return 0;
			}
		}
		dllAddress = Tools::InjectedDllAddress(hProcess, DLLNAME);

		if (button == BTN_LOAD)
		{
			if (dllAddress != NULL)
			{
				MessageBox(NULL, L"Dll已经注入,无需重复注入", L"错误", 0);
			}
			else
			{
				std::wstring fullPath = exeFolder + DLLNAME;
				if (!fullPath.empty())
				{
					if (!Tools::isFileExists_fopen(fullPath))
					{
						WCHAR buff[1024] = { 0 };
						wsprintfW(buff, L"%s不存在", fullPath.c_str());
						MessageBox(NULL, buff, L"错误", 0);
					}
					else
					{
						DWORD addr = Tools::InjectDll(hProcess, fullPath);
						if (addr == NULL)
						{
							MessageBox(NULL, L"Dll注入失败,重启微信后重试", L"错误", 0);
						}
						else
						{
							MessageBox(NULL, L"Dll注入完成", L"成功", 0);
						}
					}
				}
				else
				{
					MessageBox(NULL, L"绝对路径转换失败", L"错误", 0);
				}
			}
		}
		else if (button == BTN_UNLOAD)
		{
			if (dllAddress != NULL)
			{
				client.ExitLoop();
				if (Tools::FreeDll(hProcess, dllAddress))
				{
					dllAddress = NULL;
					MessageBox(NULL, L"Dll卸载完成", L"成功", 0);
				}
			}
			else
			{
				MessageBox(NULL, L"Dll在进程中不存在,无需重再次卸载", L"错误", 0);
			}
		}
		else if (button == BTN_OPENCONSOLE)
		{
			// 打开socket通信界面
			consoleHandle = Tools::FindDialog(L"仅限用于发送公告与通知,不得用于违法用途");

			if (consoleHandle == NULL)
			{
				DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)SocketDlgproc);
			}
			else
			{
				ShowWindow(consoleHandle, SW_SHOWNORMAL);
			}

		}
	}

	return 0;
}

INT_PTR SocketDlgproc(HWND handle, UINT msg, WPARAM button, LPARAM unnamedParam4)
{
	if (msg == WM_INITDIALOG)
	{
		consoleHandle = handle;

		client.SetEditText(handle, EDIT_DELAYSECOND, L"5");
	}
	else if (msg == WM_CLOSE)
	{
		consoleHandle = NULL;
		EndDialog(handle, 0);
	}
	else if (msg == WM_COMMAND)
	{
		processId = Tools::GetProcessIdbyName(PROCESSNAME);
		if (processId == NULL)
		{
			MessageBox(NULL, L"微信已退出,请重启微信后重试", L"错误", 0);
			return 0;
		}
		else
		{
			hProcess = Tools::GetProcessHandle(processId);
			if (hProcess == NULL)
			{
				MessageBox(NULL, L"打开进程失败,重启微信后重试", L"错误", 0);
				return 0;
			}
		}
		dllAddress = Tools::InjectedDllAddress(hProcess, DLLNAME);
		switch (button)
		{
		case BTN_SELECT_WXID:
			client.SetWxidList(handle);
			break;
		case BTN_SELECT_IMAGE:
			client.SetImagePath(handle);
			break;
		case BTN_SELECT_FILE:
			client.SetFilePath(handle);
			break;
		case BTN_ADDRESS:
			if (dllAddress == NULL)
			{
				MessageBox(NULL, L"dll已卸载,无法完成操作", L"错误", 0);
			}
			else
			{
				client.OutputAddressBook(exeFolder + L"微信通讯录.txt");
			}
			break;
		case BTN_SEND:
			if (dllAddress == NULL)
			{
				MessageBox(NULL, L"dll已卸载,无法完成操作", L"错误", 0);
			}
			else
			{
				client.SendAllTypeMessage(handle);
			}
			break;
		default:
			break;
		}
	}
	return 0;
}

RSA GetRSAInstance()
{
	BigInteger n("FF9F2A5C8CADD4568172FE5216C6D26F88068447D8BA846F00BD42DEEBA87C3");
	BigInteger e("10001");
	BigInteger d("6EB4A12509EC32BCF03D3C359315B2824C07FCBEB27AB66139134363651D9C1");
	RSAPair pair(n, e, d);
	RSA rsa;
	rsa.SetPair(pair);
	return rsa;
}

std::string GetBoardId()
{
	char computerid[100] = { 0 };
	Tools::GetBaseBoardByCmd(computerid);
	return Tools::CharStringToHexString(computerid);
}

std::string GetCheckFileInfo(std::wstring& filePath)
{
	std::ostringstream stream;
	std::string content;

	if (Tools::isFileExists_fopen(filePath))
	{
		//打开文件获取验证码
		std::ifstream file;
		file.open(filePath);
		stream << file.rdbuf();
		content = stream.str();
		file.close();
		return content;
	}
	else
	{
		return std::string();
	}
}
