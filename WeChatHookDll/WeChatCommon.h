#pragma once


DWORD moduleBaseAddr = (DWORD)LoadLibrary(L"WeChatWin.dll");

struct WeChatStr
{

	const WCHAR* pStr;
	int len;
	int maxLen;
	int fill[2] = { 0 };


	WeChatStr(const WCHAR* pStr, int len, int maxLen)
	{
		this->pStr = pStr;
		this->len = len;
		this->maxLen = maxLen;
	}
};

struct WeChatUserInfo
{
	WeChatUserInfo* current;
	WeChatUserInfo* front;
	WeChatUserInfo* next;

	WeChatStr* wxid;
	WeChatStr* name;

	WeChatUserInfo(DWORD address)
	{
		current = (WeChatUserInfo*)address;
		front = (WeChatUserInfo*)*(DWORD*)address;
		next = (WeChatUserInfo*)*(DWORD*)(address + 0x4);

		wxid = (WeChatStr*)(address + 0x30);


		DWORD nameAddress = address + 0x78;
		DWORD nameValue = *(DWORD*)nameAddress;
		name = (WeChatStr*)nameAddress;
		if (nameValue == NULL)
		{
			nameAddress = address + 0x8C;
			nameValue = *(DWORD*)nameAddress;
			name = (WeChatStr*)nameAddress;
		}

		if (Islegal() && name->pStr == NULL)
		{
			name->pStr = L"未能获取到昵称";
		}
	}

	bool Islegal()
	{
		// 判断call是否存在
		DWORD callAddr = (DWORD)current + 0x20;
		DWORD callValue = *(DWORD*)callAddr;
		return callValue == moduleBaseAddr + 0x1E8EFC8;
	}

	void Print()
	{
		WCHAR buff[0x200] = { 0 };
		wsprintf(buff, L"current:%p \nfront:%p \nnext:%p \nwxid:%s \nname:%s", current, front, next, wxid->pStr, name->pStr);
		MessageBox(NULL, buff, L"", 0);
	}

};

