#include "framework.h"
#include "HookClient.h"
#include "Tools.h"
#include "resource.h"



void HookClient::StartUp()
{
	client.StartUp();
}


void HookClient::SendAllTypeMessage(HWND handle)
{
	EnableWindow(GetDlgItem(handle, BTN_SEND), false);
	sendInterval = std::stoi(GetEditText(handle, EDIT_DELAYSECOND)) * 1000;
	text = GetEditText(handle, EDIT_TEXT);
	size_t index = text.find(L'\r');
	while (index != std::wstring::npos)
	{
		text.replace(index, 1, L"");
		index = text.find(L'\r');
	}

	for (size_t i = 0; i < wxidList.size(); i++)
	{
		SendTextMessage(wxidList[i], text);
		SendImageMessage(wxidList[i], image);
		SendFileMessage(wxidList[i], file);
		Sleep(sendInterval);
	}
	EnableWindow(GetDlgItem(handle, BTN_SEND), true);
}

void HookClient::SetWxidList(HWND handle)
{
	SetEditText(handle, PATH_WXID, Tools::BrowserFileWindow(L"文本文档(*.txt)\0*.txt\0"));
	wxidFile.open(GetEditText(handle, PATH_WXID));
	wxidStream << wxidFile.rdbuf();
	wxidString = wxidStream.str();
	wxidStream.str(L"");
	wxidFile.close();
	wxidList = Tools::split(wxidString, L'\n');
}


void HookClient::SetImagePath(HWND handle)
{
	SetEditText(handle, PATH_IMAGE, Tools::BrowserFileWindow(L"图片文件(*.jpg;*.jpeg;*.png;*.bmp;)\0*.jpg;*.jpeg;*.png;*.bmp;\0"));
	image = GetEditText(handle, PATH_IMAGE);
}

void HookClient::SetFilePath(HWND handle)
{
	SetEditText(handle, PATH_FILE, Tools::BrowserFileWindow(L"所有文件(*.*)\0*.*\0"));
	file = GetEditText(handle, PATH_FILE);
}

void HookClient::SendTextMessage(std::wstring wxid, std::wstring message)
{
	if (message.empty())
	{
		return;
	}
	WCHAR* receiver = (WCHAR*)wxid.c_str();
	WCHAR* info = (WCHAR*)message.c_str();

	WeChatMessage msg(receiver, info, E_MessageType::Text);
	client.SendSoketMessage(msg);
}

void HookClient::SendFileMessage(std::wstring wxid, std::wstring filePath)
{
	if (filePath.empty())
	{
		return;
	}
	WCHAR* receiver = (WCHAR*)wxid.c_str();
	WCHAR* info = (WCHAR*)filePath.c_str();
	WeChatMessage msg(receiver, info, E_MessageType::File);
	client.SendSoketMessage(msg);
}

void HookClient::SendImageMessage(std::wstring wxid, std::wstring imagePath)
{
	if (imagePath.empty())
	{
		return;
	}
	WCHAR* receiver = (WCHAR*)wxid.c_str();
	WCHAR* info = (WCHAR*)imagePath.c_str();
	WeChatMessage msg(receiver, info, E_MessageType::Image);
	client.SendSoketMessage(msg);
}

void HookClient::ExitLoop()
{
	WCHAR* receiver = (WCHAR*)L"";
	WCHAR* info = (WCHAR*)L"";
	WeChatMessage msg(receiver, info, E_MessageType::Exit);
	client.SendSoketMessage(msg);
}

void HookClient::OutputAddressBook(std::wstring fullPath)
{
	if (fullPath.empty())
	{
		return;
	}
	WCHAR* receiver = (WCHAR*)L"";
	WCHAR* info = (WCHAR*)fullPath.c_str();
	WeChatMessage msg(receiver, info, E_MessageType::AddressBook);
	client.SendSoketMessage(msg);
}

std::wstring HookClient::GetEditText(HWND handle, int nIDDlgItem)
{
	WCHAR buffer[1024] = { 0 };
	GetDlgItemText(handle, nIDDlgItem, buffer, sizeof(buffer));
	return std::wstring(buffer);
}

void HookClient::SetEditText(HWND handle, int nIDDlgItem, std::wstring path)
{
	SetDlgItemText(handle, nIDDlgItem, path.c_str());
}

bool HookClient::IsServerOpend()
{
	DWORD processId = Tools::GetProcessIdbyName(L"WeChat.ext");
	if (processId == NULL)
	{
		return false;
	}
	HANDLE hProcess = Tools::GetProcessHandle(processId);
	if (hProcess == NULL)
	{
		MessageBox(NULL, L"打开进程失败", L"错误", 0);
		return false;
	}
	DWORD address = Tools::InjectedDllAddress(hProcess, L"WeChatDll.dll");
	return address == NULL ? false : true;
}
