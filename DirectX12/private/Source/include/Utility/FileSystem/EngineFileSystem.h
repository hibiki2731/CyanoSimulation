#pragma once
#include <string>
class EngineFileSystem
{
public:
	static void initialize(const std::string& rootPath) {
		mRootPath = rootPath;
	}

	static std::string getEngineFilePath(const std::string& relativePath) {
		//ルートパスが初期化されていなければアサート
		assert(mRootPath != "");
		return mRootPath + relativePath;
	}

private:
	inline static std::string mRootPath;
};

