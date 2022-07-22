
#include "pch.h"
#include "SocketSever.h"
#include "WeChatSendMessage.h"
#include "Tools.h"


bool SocketSever::StartUp()
{
	//��ʼ��WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;

	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		MessageBoxW(NULL, L"socket����ʧ��,������������", L"����", 0);
		return false;
	}

	//�����׽���  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		std::cout << "create socket error !" << std::endl;
		return false;
	}

	//��IP�Ͷ˿�  
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		std::cout << "bind error !" << std::endl;
		return false;
	}

	//��ʼ����  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		std::cout << "listen error !" << std::endl;
		return false;
	}

	//�ж���Ч
	isUseable = CheckValidDate();
	//std::cout << "��Ч�ж�:" << isUseable << std::endl;

	this->slisten = slisten;

	return true;
}

void SocketSever::SendTo(SOCKET sClient, WeChatMessage& msg)
{
	//���ṹ��תΪ�ַ�����
	char buffer[0x1024] = { 0 };
	memcpy_s(buffer, 0x1024, &msg, sizeof(msg));

	/*
	send()������������ָ����socket�����Է�����
	int send(int s, const void * msg, int len, unsigned int flags)
		sΪ�ѽ��������ӵ�socket��msgָ���������ݣ�len��Ϊ���ݳ��ȣ�����flagsһ����0
		�ɹ��򷵻�ʵ�ʴ��ͳ�ȥ���ַ�����ʧ�ܷ���-1������ԭ�����error
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

	//ѭ����������  
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	while (true)
	{
		std::cout << "����...�ȴ�����" << std::endl;;
		sClient = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			std::cout << "accept error !" << std::endl;
			continue;
		}

		std::cout << "���ܵ�һ�����ӣ�" << inet_ntoa(remoteAddr.sin_addr) << std::endl;

		if (isUseable)
		{
			//���ݽ��յ�����ִ�в�ͬ�ĺ���
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
				//MessageBox(NULL, L"����֮�����Ϣ����", L"����", 0);
				break;
			}
		}



		//��������  
		const char* sendData = "��ã�TCP�ͻ��ˣ�\n";
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
