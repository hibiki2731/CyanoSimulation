#pragma once
#include <string>

namespace Utility
{
	std::wstring stringToWString(const std::string& str); 
	std::string wstringToString(const std::wstring& wstr);
}
