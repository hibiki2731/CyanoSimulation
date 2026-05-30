#include "TextureLoader.h"
#include <thread>

void TextureLoader::loadTextureAsync(TextureData& outTexture, const std::string& filePath)
{
	//状態を読み込み中に変更
	outTexture.state = TextureState::Loading; 



}
