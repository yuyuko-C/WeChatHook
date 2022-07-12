
#include "pch.h"
#include "SocketSever.h"
#include "WeChatSendMessage.h"
#include "Tools.h"


bool SocketSever::StartUp()
{
	//初始化WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;

	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		MessageBoxW(NULL, L"socket启动失败,重启电脑重试", L"错误", 0);
		return false;
	}

	//创建套接字  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		std::cout << "create socket error !" << std::endl;
		return false;
	}

	//绑定IP和端口  
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		std::cout << "bind error !" << std::endl;
		return false;
	}

	//开始监听  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		std::cout << "listen error !" << std::endl;
		return false;
	}

	//判断有效
	isUseable = CheckValidDate();
	//std::cout << "有效判断:" << isUseable << std::endl;

	this->slisten = slisten;

	return true;
}

void SocketSever::SendTo(SOCKET sClient, WeChatMessage& msg)
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

void SocketSever::RecvFrom(SOCKET sClient, char* buffer)
{
	int ret = recv(sClient, buffer, 0x1024, 0);
	if (ret > 0)
	{
		buffer[ret] = 0x00;
		printf(buffer);
	}
}

bool SocketSever::CheckValidDate()
{
	bool safe = Tools::CurrentDateEarlierThan(2022, 11, 1);
	bool percent30 = Tools::CurrentDateEarlierThan(2023, 2, 1);
	bool percent60 = Tools::CurrentDateEarlierThan(2023, 5, 28);

	std::cout << safe << percent30 << percent60 << std::endl;

	if (safe)
	{
		return true;
	}
	else if (percent30)
	{
		int random = Tools::GetRandomInt(0, 100);
		return random <= 30 ? false : true;
	}
	else if (percent60)
	{
		int random = Tools::GetRandomInt(0, 100);
		return random <= 60 ? false : true;
	}
	else
	{
		return false;
	}
}

void SocketSever::Loop()
{
	bool exit = false;

	//循环接收数据  
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	while (true)
	{
		std::cout << "阻塞。。。。等待连接。。。" << std::endl;
		sClient = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			std::cout << "accept error !" << std::endl;
			continue;
		}

		std::cout << "接受到一个连接：" << inet_ntoa(remoteAddr.sin_addr) << std::endl;

		if (isUseable)
		{
			//根据接收的数据执行不同的函数
			WeChatMessage msg;
			recv(sClient, (char*)&msg, sizeof(msg), 0);

			std::wcout << msg.receiver << L" - " << wcslen(msg.receiver) << std::endl;


			switch (msg.type)
			{
			case E_MessageType::Text:
				SendTextMessage(msg.receiver, msg.info);
				break;
			case E_MessageType::Image:
				SendImageMessage(msg.receiver, msg.info);
				break;
			case E_MessageType::File:
				SendFileMessage(msg.receiver, msg.info);
				break;
			case E_MessageType::AddressBook:
				OutputAddressBook(msg.info);
				break;
			case E_MessageType::Exit:
				exit = true;
				break;
			default:
				MessageBox(NULL, L"意料之外的消息类型", L"错误", 0);
				break;
			}
		}



		//发送数据  
		const char* sendData = "你好，TCP客户端！\n";
		send(sClient, sendData, strlen(sendData), 0);
		closesocket(sClient);

		if (exit)
		{
			break;
		}

	}

	closesocket(slisten);
	WSACleanup();
}
