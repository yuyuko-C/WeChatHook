#pragma once

#include<winsock2.h>
#include<iostream>
#include<string>

#pragma comment(lib, "ws2_32.lib")


enum class E_MessageType
{
	Text, Image, File, AddressBook, Exit
};


struct WeChatMessage
{
	WCHAR receiver[40];
	WCHAR info[2048];
	E_MessageType type;

	WeChatMessage()
	{
		memset(receiver, 0, sizeof(receiver));
		memset(info, 0, sizeof(info));
		type = E_MessageType::Text;
	}

	WeChatMessage(WCHAR* _receiver, WCHAR* _info, E_MessageType _type)
	{
		memset(receiver, 0, sizeof(receiver));
		memset(info, 0, sizeof(info));
		type = _type;

		memcpy_s(receiver, sizeof(receiver), _receiver, wcslen(_receiver) * 2);
		memcpy_s(info, sizeof(info), _info, wcslen(_info) * 2);

	}
};

class SocketSever
{
public:

	bool StartUp();
	void Loop();

private:
	void SendTo(SOCKET sClient, WeChatMessage& msg);
	void RecvFrom(SOCKET sClient, char* buffer);
	bool CheckValidDate();

	SOCKET slisten = 0;
	bool isUseable;
};

