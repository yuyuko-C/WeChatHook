// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "pch.h"

#include "SocketSever.h"
#include "Tools.h"
#include <Windows.h>
#include <WinUser.h>
#include <Windowsx.h>
#include <string>
#include <iostream> 
#include <winsock2.h>
#include "WeChatSendMessage.h"

#pragma comment(lib,"ws2_32.lib")  
#define _WINSOCK_DEPRECATED_NO_WARNINGS

DWORD ThreadSocket();

DWORD WeChatWin = (DWORD)LoadLibrary(L"WeChatWin.dll");
SocketSever server;


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	// ÿ����Ϣѭ������ִ�е�����

	// �����β
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//Tools::OpenConsole();
		if (server.StartUp())
		{
			std::cout << "׼������" << std::endl;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSocket, NULL, 0, NULL);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



// �����socket�߳�
DWORD ThreadSocket()
{
	server.Loop();
	return 0;
}
