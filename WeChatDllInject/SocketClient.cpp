
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SocketClient.h"


bool SocketClient::StartUp()
{		
	// �����첽socketͨ��
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		MessageBoxW(NULL, L"socket����ʧ��,������������", L"����", 0);
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
		//����ʧ�� 
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


void SocketClient::RecvFrom(SOCKET sClient, char* buffer)
{
	//��������  
	int ret = recv(sClient, buffer, 0x1024, 0);
	if (ret > 0)
	{
		buffer[ret] = 0x00;
		printf(buffer);
	}
}
