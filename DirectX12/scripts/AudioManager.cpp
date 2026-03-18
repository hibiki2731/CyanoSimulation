#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <expected>
#include <iostream>
#include <bit>
#include <fstream>
#include <algorithm>
#include "AudioManager.h"
#include "MyUtility.h"
#include "json.hpp"

AudioManager::AudioManager() {
	//リトルエンディアンとビッグエンディアンそれぞれに適したfourccに設定
	if constexpr (std::endian::native == std::endian::little) {
		fourccRIFF = 'FFIR';
		fourccFMT  = ' tmf';
		fourccDATA = 'atad';
		fourccWAVE = 'EVAW';
	}
	else if constexpr (std::endian::native == std::endian::big) {
		fourccRIFF = 'RIFF';
		fourccFMT  = 'fmt ';
		fourccDATA = 'data';
		fourccWAVE = 'WAVE';
	}
	mCurrentBGM = nullptr;

	initXAudio();
	loadSoundFiles();
}

AudioManager::~AudioManager()
{
	mSoundInstances.clear();
}

void AudioManager::initXAudio()
{
	//COMの初期化
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) std::cerr << "COMの初期化失敗\n";

	//XAudio2エンジンの初期化
	hr = XAudio2Create(mXAudio.ReleaseAndGetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr)) std::cerr << "XAudio2エンジンの初期化失敗\n";

	//マスタリングボイスの初期化
	hr = mXAudio->CreateMasteringVoice(&mMasteringVoice);
	if (FAILED(hr)) std::cerr << "マスタリングボイスの初期化失敗\n";
}

void AudioManager::loadSoundFiles()
{
	//サウンドデータファイルを開く
	std::fstream file("assets/data/soundData.json");
	nlohmann::json json;
	file >> json;

	//jsonファイルに列挙したサウンドデータを読み込む
	for (auto soundJson : json) {
		SoundData soundData;
		loadWAVFile(soundJson["filePath"], soundData);
		mSoundDataList[soundJson["id"]] = std::move(soundData);
	}
}

void AudioManager::playBGM(std::string soundID)
{
	finishBGM();

	//サウンドデータを取得
	auto iter = mSoundDataList.find(soundID);
	if (iter == mSoundDataList.end()) return;
	SoundData& soundData = iter->second;

	//バッファーを作成
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = static_cast<UINT32>(soundData.audioData.size());	//バッファーのバイト数
	buffer.pAudioData = soundData.audioData.data();							//オーディオデータのポインタ
	buffer.Flags = XAUDIO2_END_OF_STREAM;									//このバッファの後に他のバッファが来ない
	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;								//無限ループ

	auto soundInstance = std::make_unique<SoundInstance>();
	HRESULT hr = mXAudio->CreateSourceVoice(&soundInstance->mSourceVoice, reinterpret_cast<WAVEFORMATEX*>(&soundData.waveFormat));
	if (FAILED(hr)) std::cerr << "ソースボイスの作成に失敗/n";

	soundInstance->mSourceVoice->SetVolume(1.0f);	//元音源と同じ音量
	hr = soundInstance->mSourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr)) std::cerr << "バッファの送信失敗/n";

	soundInstance->mSourceVoice->Start(0);

	soundInstance->isLoop = true;
	mCurrentBGM = soundInstance.get();
	mSoundInstances.emplace_back(std::move(soundInstance));
}

void AudioManager::playSE(std::string soundID)
{
	//再生が終了したボイスをクリアする
	clearFinishedSounds();

	//サウンドデータを取得
	auto iter = mSoundDataList.find(soundID);
	if (iter == mSoundDataList.end()) return;
	SoundData& soundData = iter->second;

	//バッファーを作成
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = static_cast<UINT32>(soundData.audioData.size());	//バッファーのバイト数
	buffer.pAudioData = soundData.audioData.data();							//オーディオデータのポインタ
	buffer.Flags = XAUDIO2_END_OF_STREAM;									//このバッファの後に他のバッファが来ない

	auto soundInstance = std::make_unique<SoundInstance>();
	HRESULT hr = mXAudio->CreateSourceVoice(&soundInstance->mSourceVoice, reinterpret_cast<WAVEFORMATEX*>(&soundData.waveFormat));
	if (FAILED(hr)) std::cerr << "ソースボイスの作成に失敗/n";

	soundInstance->mSourceVoice->SetVolume(1.0f);	//元音源と同じ音量
	hr = soundInstance->mSourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr)) std::cerr << "バッファの送信失敗/n";

	soundInstance->mSourceVoice->Start(0);

	mSoundInstances.emplace_back(std::move(soundInstance));

}

void AudioManager::stopBGM()
{
	if (mCurrentBGM) {
		mCurrentBGM->mSourceVoice->Stop();
	}
}

void AudioManager::stopAllSounds()
{
	for (auto& soundInst : mSoundInstances) {
		soundInst->mSourceVoice->Stop();
	}
}

void AudioManager::finishBGM()
{
	if (mCurrentBGM) {
		mCurrentBGM->mSourceVoice->Stop();
		mCurrentBGM->mSourceVoice->DestroyVoice();
		mCurrentBGM->mSourceVoice = nullptr;
		auto iter = std::find_if(mSoundInstances.begin(), mSoundInstances.end(),
			[this](const std::unique_ptr<SoundInstance>& ptr) {
				return ptr.get() == this->mCurrentBGM;
			});
		std::iter_swap(iter, mSoundInstances.end() - 1);
		mSoundInstances.pop_back();
	}
}

void AudioManager::finishAllSounds()
{
	for (auto& soundInst : mSoundInstances) {
		soundInst->mSourceVoice->Stop();
		soundInst->mSourceVoice->DestroyVoice();
		soundInst->mSourceVoice = nullptr;
	}
	mSoundInstances.clear();

}

void AudioManager::clearFinishedSounds()
{
	std::vector<std::unique_ptr<SoundInstance>> deleteInst;
	//再生が終了したボイスを一次配列に追加
	for (auto& soundInst : mSoundInstances) {
		//ソースボイスがnullptrの場合、もしくはループ再生している音源はスルー
		if (!soundInst->mSourceVoice || soundInst->isLoop) continue;
		XAUDIO2_VOICE_STATE state;
		soundInst->mSourceVoice->GetState(&state);

		if (state.BuffersQueued == 0) {
			soundInst->mSourceVoice->DestroyVoice();
			soundInst->mSourceVoice = nullptr;
			deleteInst.emplace_back(std::move(soundInst));
		}
	}

	//元配列に残ったnullptrを削除
	std::erase_if(mSoundInstances, [](const std::unique_ptr<SoundInstance>& sound) {
		return sound == nullptr;
	});




}

HRESULT AudioManager::findChunk(HANDLE hFile, DWORD targetFourcc, DWORD& chunkDataSize, DWORD& chunkDataPosition)
{
	HRESULT hr = S_OK;
	//読み込むファイルの先頭にハンドルをセット
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, nullptr, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	
    DWORD chunkType;
    DWORD dataSize;
    DWORD RIFFDataSize = 0;
    DWORD fileType;
    DWORD bytesRead = 0;
    DWORD offset = 0;

	//順にチャンクを読み込み、目的のチャンクの位置を探す
	while (hr == S_OK) {
		
		DWORD read;
		//チャンクの種類を読み込む
		if (0 == ReadFile(hFile, &chunkType, sizeof(DWORD), &read, NULL))
			return HRESULT_FROM_WIN32(GetLastError());
		bytesRead += read;

		//チャンクのサイズを読み込む
		if (0 == ReadFile(hFile, &dataSize, sizeof(DWORD), &read, NULL))
			return HRESULT_FROM_WIN32(GetLastError());
		bytesRead += read;

		//チャンクがRIFFだった場合
		if (chunkType == fourccRIFF) {
			RIFFDataSize = dataSize;	//チャンクデータサイズにファイルデータサイズが入っているため、移す
			dataSize = 4;
			//ファイルの種類を読み込む
			if (0 == ReadFile(hFile, &fileType, sizeof(DWORD), &read, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			bytesRead += read;
		}
		//チャンクがRIFF以外の時
		else{
			//チャンクデータサイズ分ハンドルを移動させる
            if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, dataSize, NULL, FILE_CURRENT ) )
            return HRESULT_FROM_WIN32( GetLastError() );            
        }

		offset += sizeof(DWORD) * 2; //fileTypeとfileSize自身のサイズ分オフセットを追加

		//探していたチャンクの種類と一致した場合、チャンクのデータサイズとデータの位置を返す
		if (chunkType == targetFourcc) {
			chunkDataSize = dataSize;
			chunkDataPosition = offset;
			return S_OK;
		}

		offset += dataSize; //dataのサイズ分オフセットを追加
		bytesRead += dataSize;
		//ファイルの最後まで読み込んだ場合、失敗
		if (bytesRead >= RIFFDataSize) return S_FALSE;

	}

	return S_OK;
}

HRESULT AudioManager::readChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset)
{
    HRESULT hr = S_OK;
	//目標のチャンクデータの位置にハンドルを移動
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, bufferOffset, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );

	//バッファーサイズ分、データを読み込む
    if( 0 == ReadFile( hFile, buffer, bufferSize, nullptr, NULL))
        hr = HRESULT_FROM_WIN32( GetLastError() );

	//コンピュータがリトルエンディアンの場合wavのデータと一致するので、そのままbufferにデータを移す
	if constexpr (std::endian::native == std::endian::little) {
		return hr;
	}
	//ビッグエンディアンの場合、バイトの並びをひっくり返す
	else if constexpr (std::endian::native == std::endian::big) {
		// 安全対策：バッファのサイズが4バイトの倍数かチェック
		if (bufferSize % sizeof(uint32_t) != 0) {
			std::cerr << "バッファサイズが4の倍数ではありません。\n";
			return;
		}

		//バイト配列の先頭ポインタを、32ビット整数のポインタに強制変換
		uint32_t* pData = reinterpret_cast<uint32_t*>(buffer);

		//32ビット整数の個数を計算
		size_t count = bufferSize / sizeof(uint32_t);

		//全要素をループしてスワップ
		for (size_t i = 0; i < count; ++i) {
			pData[i] = std::byteswap(pData[i]);
		}
	}


    return hr;
}

HRESULT AudioManager::loadWAVFile(const std::string& filePath, SoundData& outputData)
{
	const std::wstring src = Utility::stringToWString(filePath);
	//wavファイルを開く
	HANDLE hFile = CreateFile(
		src.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (INVALID_HANDLE_VALUE == hFile)
		return HRESULT_FROM_WIN32(GetLastError());

	//ハンドルをファイルの始めにセット
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD chunkSize;
	DWORD chunkPosition;
	//ファイルがwavファイルかどうか確認
	findChunk(hFile, fourccRIFF, chunkSize, chunkPosition);
	DWORD filetype;
	readChunkData(hFile, &filetype, sizeof(DWORD), chunkPosition);

	if (filetype != fourccWAVE)
		return S_FALSE;

	//'fmt'チャンクからWAVFORMATEXTENSIBLEのデータを取得
	findChunk(hFile, fourccFMT, chunkSize, chunkPosition);
	readChunkData(hFile, &outputData.waveFormat, chunkSize, chunkPosition );

	
	//'data'チャンクから内容をバッファーに書き込む
	findChunk(hFile, fourccDATA, chunkSize, chunkPosition);
	outputData.audioData.resize(static_cast<size_t>(chunkSize));
	readChunkData(hFile, static_cast<void*>(outputData.audioData.data()), chunkSize, chunkPosition);

	return S_OK;
}

