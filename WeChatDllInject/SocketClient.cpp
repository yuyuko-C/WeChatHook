
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SocketClient.h"


bool SocketClient::StartUp()
{		
	// 启动异步socket通信
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		MessageBoxW(NULL, L"socket启动失败,重启电脑重试", L"错误", 0);
		return false;
	}
	return true;
}

void SocketClient::SendSoketMessage(WeChatMessage& msg)
{
	SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		std::cout << "invalid socket!" << std::endl;
		return;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sClient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		//连接失败 
		std::cout << "connect error !" << std::endl;
		closesocket(sClient);
		return;
	}

	send(sClient, (char*)&msg, sizeof(msg), 0);

	char buffer[0x1024] = { 0 };
	RecvFrom(sClient, buffer);
	closesocket(sClient);

}


void SocketClient::SendTo(SOCKET sClient, WeChatMessage& msg)
{
	//将结构体转为字符数组
	char buffer[0x1024] = { 0 };
	memcpy_s(buffer, 0x1024, &msg, sizeof(msg));

	/*
	send()用来将数据由指定的socket传给对方主机
	int send(int s, const void * msg, int len, unsigned int flags)
		s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
		成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error
	*/
	send(sClient, buffer, sizeof(buffer), 0);

}


void SocketClient::RecvFrom(SOCKET sClient, char* buffer)
{
	//接收数据  
	int ret = recv(sClient, buffer, 0x1024, 0);
	if (ret > 0)
	{
		buffer[ret] = 0x00;
		printf(buffer);
	}
}
