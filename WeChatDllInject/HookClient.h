#pragma once
#include "SocketClient.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class HookClient
{
public:

	void StartUp();


	void SendAllTypeMessage(HWND handle);
	void SetWxidList(HWND handle);
	void SetImagePath(HWND handle);
	void SetFilePath(HWND handle);

	void SendTextMessage(std::wstring wxid, std::wstring message);
	void SendFileMessage(std::wstring wxid, std::wstring filePath);
	void SendImageMessage(std::wstring wxid, std::wstring imagePath);
	void ExitLoop();
	void OutputAddressBook(std::wstring fullPath);

	std::wstring GetEditText(HWND handle, int nIDDlgItem);
	void SetEditText(HWND handle, int nIDDlgItem, std::wstring path);

	bool IsServerOpend();


private:

	SocketClient client;
	int sendInterval = 0;
	std::wstring text, image, file;	
	std::wfstream wxidFile;
	std::wostringstream  wxidStream;
	std::wstring wxidString;
	std::vector<std::wstring> wxidList;

};

