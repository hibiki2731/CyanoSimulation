#include "MyUtility.h"
#include <windows.h>

std::wstring Utility::stringToWString(const std::string& str)
{
	//文字列が空の場合は空のwstringを返す
	if (str.empty()) {
		return std::wstring();
	}
	//マルチバイト文字列をワイド文字列に変換するためのバッファサイズを取得
	int wSize = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);

	//ワイド文字列を格納するためのwstringオブジェクトを作成
	std::wstring wstr(wSize, 0);

	//マルチバイト文字列をワイド文字列に変換してwstringオブジェクトに格納
	MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstr[0], wSize);

	return wstr;
}
