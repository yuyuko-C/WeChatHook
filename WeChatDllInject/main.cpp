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
RSA GetRSAInstance();// ��ʼ��rsa
std::string GetBoardId();// ��ȡ��������ID��ʮ������д��
std::string GetCheckFileInfo(std::wstring& filePath);// ��ȡ��֤�ļ��ַ���


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
	std::cout << "����ΨһID:";
	std::string in;
	std::cin >> in;
	std::cout << rsa.encryptByPublic(in) << std::endl;
	system("pause");
#else
	//Tools::OpenConsole();
	//ִ�������֤
	std::wstring path = L"./6EB4A12509EC32BCF03D3C359315B282.db";
	std::string hexboard = GetBoardId();
	std::string content = GetCheckFileInfo(path);
	if (content.empty())
	{
		MessageBox(NULL, Tools::String2Wstring(hexboard).c_str(), L"", 0);
	}
	else
	{
		//��֤ͨ��
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
			MessageBox(NULL, L"΢�����˳�,������΢�ź�����", L"����", 0);
			return 0;
		}
		else
		{
			hProcess = Tools::GetProcessHandle(processId);
			if (hProcess == NULL)
			{
				MessageBox(NULL, L"�򿪽���ʧ��,����΢�ź�����", L"����", 0);
				return 0;
			}
		}
		dllAddress = Tools::InjectedDllAddress(hProcess, DLLNAME);

		if (button == BTN_LOAD)
		{
			if (dllAddress != NULL)
			{
				MessageBox(NULL, L"Dll�Ѿ�ע��,�����ظ�ע��", L"����", 0);
			}
			else
			{
				std::wstring fullPath = exeFolder + DLLNAME;
				if (!fullPath.empty())
				{
					if (!Tools::isFileExists_fopen(fullPath))
					{
						WCHAR buff[1024] = { 0 };
						wsprintfW(buff, L"%s������", fullPath.c_str());
					}
					else
					{
						DWORD addr = Tools::InjectDll(hProcess, fullPath);
						if (addr == NULL)
						{
							MessageBox(NULL, L"Dllע��ʧ��,����΢�ź�����", L"����", 0);
						}
						else
						{
							MessageBox(NULL, L"Dllע�����", L"�ɹ�", 0);
						}
					}
				}
				else
				{
					MessageBox(NULL, L"����·��ת��ʧ��", L"����", 0);
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
					MessageBox(NULL, L"Dllж�����", L"�ɹ�", 0);
				}
			}
			else
			{
				MessageBox(NULL, L"Dll�ڽ����в�����,�������ٴ�ж��", L"����", 0);
			}
		}
		else if (button == BTN_OPENCONSOLE)
		{
			// ��socketͨ�Ž���
			consoleHandle = Tools::FindDialog(L"�������ڷ��͹�����֪ͨ,��������Υ����;");

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

		client.SetEditText(handle, EDIT_DELAYSECOND, L"3");
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
			MessageBox(NULL, L"΢�����˳�,������΢�ź�����", L"����", 0);
			return 0;
		}
		else
		{
			hProcess = Tools::GetProcessHandle(processId);
			if (hProcess == NULL)
			{
				MessageBox(NULL, L"�򿪽���ʧ��,����΢�ź�����", L"����", 0);
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
				MessageBox(NULL, L"dll��ж��,�޷���ɲ���", L"����", 0);
			}
			else
			{
				client.OutputAddressBook(exeFolder + L"΢��ͨѶ¼.txt");
			}
			break;
		case BTN_SEND:
			if (dllAddress == NULL)
			{
				MessageBox(NULL, L"dll��ж��,�޷���ɲ���", L"����", 0);
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
		//���ļ���ȡ��֤��
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
