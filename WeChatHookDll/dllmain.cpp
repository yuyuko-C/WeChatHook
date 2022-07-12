// dllmain.cpp : 定义 DLL 应用程序的入口点。
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
	// 每个消息循环都会执行的区域

	// 区域结尾
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//Tools::OpenConsole();
		if (server.StartUp())
		{
			std::cout << "准备就绪" << std::endl;
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



// 服务端socket线程
DWORD ThreadSocket()
{
	server.Loop();
	return 0;
}
