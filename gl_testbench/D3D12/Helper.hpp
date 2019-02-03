#pragma once
#include <string>
#include <Windows.h>

std::wstring towstr(std::string str)
{
	std::wstring wstr;
	wchar_t buffer[1024] = { 0 };

	int length = static_cast<int>(str.size());
	int n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), length, buffer, length);
	if (n == length)
		wstr.assign(buffer, n);

	return wstr;
}

std::string tostr(std::wstring wstr)
{
	std::string str;
	char buffer[1024] = { 0 };

	int length = static_cast<int>(str.size());
	int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), length, buffer, length, NULL, NULL);
	if (n != 0)
		str.assign(buffer, n);

	return str;
}