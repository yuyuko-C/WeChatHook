#pragma once
#define DllExport   __declspec( dllexport )


#include <string>


void SendTextMessage(std::wstring wxid, std::wstring message);
void SendFileMessage(std::wstring wxid, std::wstring filePath);
void SendImageMessage(std::wstring wxid, std::wstring imagePath);
void OutputAddressBook(std::wstring fullPath);



extern "C" DllExport void _cdecl  _OutputAddressBook(std::wstring fullPath);
extern "C" DllExport void _cdecl  _SendTextMessage(std::wstring wxid, std::wstring text);
extern "C" DllExport void _cdecl  _SendFileMessage(std::wstring wxid, std::wstring filePath);
extern "C" DllExport void _cdecl  _SendImageMessage(std::wstring wxid, std::wstring imagePath);
//extern "C" DllExport void _cdecl  _GetAddressBook();