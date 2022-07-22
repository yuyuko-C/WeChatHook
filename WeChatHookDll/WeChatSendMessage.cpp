#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <shellapi.h>
#include <Commdlg.h>

#include <codecvt>
#include "WeChatSendMessage.h"
#include "WeChatCommon.h"




void SendTextMessage(std::wstring wxid, std::wstring message)
{
	// ?????????call
	DWORD sendCall = moduleBaseAddr + 0x54BB1D30 - 0x54690000;

	// ??????
	WeChatStr id = { wxid.c_str(), (int)wxid.length(),(int)wxid.length() };
	WeChatStr msg = { message.c_str(),(int)message.length(),(int)message.length() };

	// ??????
	WCHAR at[0x38] = { 0 };
	WCHAR buff[0x3B0] = { 0 };


	__asm
	{
		pushad;

		lea eax, at;
		lea edi, msg;
		push 0x1;
		push eax;
		push edi;
		lea edx, id;
		lea ecx, buff;
		call sendCall;
		add esp, 0xC;

		popad;
	}

}

void SendFileMessage(std::wstring wxid, std::wstring filePath)
{
	WCHAR buff[0x3B0] = { 0 };

	WeChatStr id = { wxid.c_str(), (int)wxid.length(),(int)wxid.length() };
	WeChatStr path = { filePath.c_str(),(int)filePath.length(),(int)filePath.length() };


	DWORD jumpCall = moduleBaseAddr + 0x52DE3B70 - 0x52D20000;
	DWORD clearCall = moduleBaseAddr + 0x53491980 - 0x52D20000;
	DWORD infoCall = moduleBaseAddr + 0x52B21980 - 0x523B0000;
	DWORD ecxCall = moduleBaseAddr + 0x547166B0 - 0x523B0000;
	DWORD combineCall = moduleBaseAddr + 0x5279D8C0 - 0x523B0000;
	DWORD sendArg1 = moduleBaseAddr + 0x565D645C - 0x54740000;
	DWORD sendArg2 = moduleBaseAddr + 0x565D645C - 0x54740000;
	DWORD sendCall = moduleBaseAddr + 0x54DE6450 - 0x54740000;


	WCHAR eaxBuff[0xA] = { 0 };

	__asm
	{
		pushad;

		call jumpCall;
		sub esp, 0x14;
		mov dword ptr ss : [ebp - 0x48] , eax;
		lea eax, eaxBuff;
		mov ecx, esp;
		push eax;
		call clearCall;
		push 0x0;
		sub esp, 0x14;
		mov edi, esp;
		mov dword ptr ds : [edi] , 0x0;
		mov dword ptr ds : [edi + 0x4] , 0x0;
		mov dword ptr ds : [edi + 0x8] , 0x0;
		mov dword ptr ds : [edi + 0xC] , 0x0;
		mov dword ptr ds : [edi + 0x10] , 0x0;
		mov eax, dword ptr ds : [edi] ;


		sub esp, 0x14;
		lea eax, path;
		mov ecx, esp;
		push eax;
		call infoCall;
		sub esp, 0x14;
		lea eax, id;
		mov ecx, esp;
		push eax;
		call infoCall;
		mov ecx, ecxCall;
		lea eax, buff;
		push eax;
		call combineCall;
		mov edi, sendArg1;
		mov ecx, sendArg2;
		call sendCall;



		popad;
	}
}

void SendImageMessage(std::wstring wxid, std::wstring imagePath)
{

	DWORD switchCall = moduleBaseAddr + 0x5236D780 - 0x522B0000;
	DWORD eaxCall = moduleBaseAddr + 0x54616630 - 0x522B0000;
	DWORD pathCall = moduleBaseAddr + 0x52A21980 - 0x522B0000;
	DWORD sendCall = moduleBaseAddr + 0x527D1640 - 0x522B0000;

	WeChatStr id = { wxid.c_str(), (int)wxid.length(),(int)wxid.length() };
	WeChatStr image = { imagePath.c_str(),(int)imagePath.length(),(int)imagePath.length() };


	WCHAR eaxBuff[0x18] = { 0 };
	WCHAR eaxBuff2[0x3B0] = { 0 };



	__asm
	{
		pushad;

		call switchCall;
		sub esp, 0x14;
		mov eax, eaxCall;
		mov dword ptr ss : [ebp - 0x48] , eax;
		lea eax, eaxBuff;
		mov ecx, esp;
		lea edi, image;
		push eax;
		call pathCall;
		mov ecx, dword ptr ss : [ebp - 0x48] ;
		lea eax, id;
		push edi;
		push eax;
		lea eax, eaxBuff2;
		push eax;
		call sendCall;

		popad;

	}
}

void OutputAddressBook(std::wstring fullPath)
{
	DWORD head = *(DWORD*)(moduleBaseAddr + 0x023668F4);
	head = *(DWORD*)(head + 0x4C);

	std::ofstream file;
	file.open(fullPath, std::ios::out);
	if (!file.is_open())
	{
		MessageBox(NULL, L"??????,??¨´???????????????????", L"????", 0);
	}

	file.imbue(std::locale("", std::locale::all));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> codev8;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> codev816;

	file << codev816.to_bytes(L"wxid,??????").c_str() << std::endl;

	WeChatUserInfo info = { head };
	do
	{
		if (info.Islegal())
		{
			file << codev816.to_bytes(info.wxid->pStr) << "\t" << codev816.to_bytes(info.name->pStr) << std::endl;
			file.flush();
		}
		info = { (DWORD)info.front };

	} while (head != (DWORD)info.current);

	file.close();



	WCHAR buffer[0x100] = { 0 };
	wsprintf(buffer, L"explorer /select, %s", fullPath.c_str());
	//MessageBox(NULL, buffer, L"", 0);
	_wsystem(buffer);

}





DllExport void _cdecl _OutputAddressBook(std::wstring fullPath)
{
	OutputAddressBook(fullPath);
}

DllExport void _cdecl _SendTextMessage(std::wstring wxid, std::wstring text)
{
	SendTextMessage(wxid, text);
}

DllExport void _cdecl _SendFileMessage(std::wstring wxid, std::wstring filePath)
{
	SendFileMessage(wxid, filePath);
}

DllExport void _cdecl _SendImageMessage(std::wstring wxid, std::wstring imagePath)
{
	SendImageMessage(wxid, imagePath);
}
