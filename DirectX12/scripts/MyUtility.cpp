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

	//変換に失敗した場合は空のwstringを返す
	if (wSize <= 0) {
		return std::wstring();
	}

	//ワイド文字列を格納するためのwstringオブジェクトを作成
	std::wstring wstr(wSize, 0);

	//マルチバイト文字列をワイド文字列に変換してwstringオブジェクトに格納
	MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstr[0], wSize);

	return wstr;
}

std::string Utility::wstringToString(const std::wstring& wstr)
{
	if (wstr.empty()) {
        return std::string();
    }

    //変換後の文字列に必要なバイト数を計算
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    
	if (sizeNeeded <= 0) {
		return std::string();
	}

    //必要なサイズのstringを確保
    std::string strTo(sizeNeeded, 0);
    
    //stringに書き込む
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), strTo.data(), sizeNeeded, NULL, NULL);
    
    return strTo;
}
