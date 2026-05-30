#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <atomic>
#include <string>

using Microsoft::WRL::ComPtr;

enum class TextureState {
	Unloaded, //テクスチャが読み込まれていない状態
	Loading,  //テクスチャが読み込み中の状態
	Loaded,   //テクスチャが読み込まれた状態
	Failed    //テクスチャの読み込みに失敗した状態
};

//テクスチャの状態とリソースを管理する構造体
struct TextureData {
	std::atomic<TextureState> state{ TextureState::Unloaded }; //テクスチャの状態。複数のスレッドからアクセスするため、std::atomicを使用
	ComPtr<ID3D12Resource> texture; //テクスチャのリソース
};

//テクスチャの読み込みを行うクラス
class TextureLoader
{
public:

	static void loadTextureAsync(TextureData& outTexture, const std::string& filePath);

};

