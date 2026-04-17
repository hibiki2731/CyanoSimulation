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
#include "stb_vorbis.c"
#include <thread>
#include <chrono>

#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

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
	//今後ハードウェアに送られる音声のボリュームを0にする
	if (mMasteringVoice) mMasteringVoice->SetVolume(0.0f);

	//ソースボイスの再生を停止する
	finishAllSounds();

	//SoundData内のデストラクタで、ソースボイスが破棄される
	mSoundDataList.clear();

	//すでにハードウェアに送られた音声が再生されるのを待つ(マスタリングボイスを破棄する前に行う)
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//マスタリングボイスを止める
	if (mMasteringVoice) {
		mMasteringVoice->DestroyVoice();
		mMasteringVoice = nullptr;
	}
	//XAudio2のエンジンを止める
	if (mXAudio) {
		mXAudio->StopEngine();
	}

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

	//SEデータを読み込む
	for (const auto& seJson : json["SE"]) {
		auto soundData = std::make_unique<SoundData>();
		int poolSize = seJson.value("poolSize", 4);
		//WAVEファイルの場合
		if (seJson["fileType"] == "wav")
			loadWAVFile(seJson["filePath"], *soundData, poolSize);
		//OGGファイルの場合
		else if (seJson["fileType"] == "ogg")
			loadOGGFile(seJson["filePath"], *soundData, poolSize);
		//MP3ファイルの場合
		else if (seJson["fileType"] == "mp3")
			loadMP3File(seJson["filePath"], *soundData, poolSize);
		mSoundDataList[seJson["id"]] = std::move(soundData);
	}
	//BGMデータを読み込む
	for (const auto& bgmJson : json["BGM"]) {
		auto soundData = std::make_unique<SoundData>();
		//WAVEファイルの場合
		if (bgmJson["fileType"] == "wav")
			loadWAVFile(bgmJson["filePath"], *soundData, 1);
		//OGGファイルの場合
		else if (bgmJson["fileType"] == "ogg")
			loadOGGFile(bgmJson["filePath"], *soundData, 1);
		//MP3ファイルの場合
		else if (bgmJson["fileType"] == "mp3")
			loadMP3File(bgmJson["filePath"], *soundData, 1);
		mSoundDataList[bgmJson["id"]] = std::move(soundData);
	}
}

void AudioManager::playBGM(const std::string& soundID)
{
	//再生が終了したボイスを再生中配列から除去する
	clearFinishedSounds();

	//サウンドデータを取得
	auto iter = mSoundDataList.find(soundID);
	if (iter == mSoundDataList.end()) return;
	SoundData& soundData = *iter->second;

	//BGMを止める
	finishBGM();

	soundData.sourceVoices[0]->SetVolume(1.0f);	//元音源と同じ音量

	//バッファの設定
	soundData.buffer.LoopCount = XAUDIO2_LOOP_INFINITE;	//ループ再生

	HRESULT hr = soundData.sourceVoices[0]->SubmitSourceBuffer(&soundData.buffer);
	if (FAILED(hr)) std::cerr << "バッファの送信失敗\n";
	
	//再生開始
	soundData.sourceVoices[0]->Start(0);

	//再生中のBGMに設定
	mCurrentBGM = soundData.sourceVoices[0];
}

IXAudio2SourceVoice* AudioManager::playSE(const std::string& soundID)
{
	//再生が終了したボイスを再生中配列から除去する
	clearFinishedSounds();

	//サウンドデータを取得
	auto iter = mSoundDataList.find(soundID);
	if (iter == mSoundDataList.end()) return nullptr;
	SoundData& soundData = *iter->second;
	
	for (auto voice : soundData.sourceVoices) {

		XAUDIO2_VOICE_STATE state;
		voice->GetState(&state);
		if (state.BuffersQueued != 0) continue;
		HRESULT hr = voice->SubmitSourceBuffer(&soundData.buffer);
		if (FAILED(hr)) std::cerr << "バッファの送信失敗\n";

		//再生開始
		voice->Start(0);

		//再生中ボイス配列に追加
		mNowPlayingVoicess.emplace_back(voice);
		return voice;
	}

	//全てのボイスが再生中の場合
	soundData.sourceVoices[0]->Stop(0);
	soundData.sourceVoices[0]->FlushSourceBuffers();
	soundData.sourceVoices[0]->SubmitSourceBuffer(&soundData.buffer);
	soundData.sourceVoices[0]->Start(0);
	return soundData.sourceVoices[0];

}

void AudioManager::pauseBGM()
{
	if (mCurrentBGM) {
		mCurrentBGM->Stop();
	}
}

void AudioManager::pauseAllSounds()
{
	for (auto voice : mNowPlayingVoicess) {
		voice->Stop();
	}
	pauseBGM();
}

void AudioManager::finishBGM()
{
	if (mCurrentBGM) {
		mCurrentBGM->Stop(0);
		mCurrentBGM->FlushSourceBuffers();
		mCurrentBGM = nullptr;
	}
}

void AudioManager::finishAllSounds()
{
	for (auto voice : mNowPlayingVoicess) {
		voice->Stop(0);
		voice->FlushSourceBuffers();
	}
	mNowPlayingVoicess.clear();

	finishBGM();
}

void AudioManager::clearFinishedSounds()
{
	//再生が終了したボイスを配列から除去
	std::erase_if(mNowPlayingVoicess,
		[](IXAudio2SourceVoice*& voice) {
			if (!voice) return false;
			XAUDIO2_VOICE_STATE state;
			voice->GetState(&state);

			return state.BuffersQueued == 0;
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

HRESULT AudioManager::loadWAVFile(const std::string& filePath, SoundData& outputData, int poolSize)
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

	WAVEFORMATEXTENSIBLE waveFormat;
	//'fmt'チャンクからWAVFORMATEXTENSIBLEのデータを取得
	findChunk(hFile, fourccFMT, chunkSize, chunkPosition);
	readChunkData(hFile, &waveFormat, chunkSize, chunkPosition );

	//'data'チャンクから内容をバッファーに書き込む
	findChunk(hFile, fourccDATA, chunkSize, chunkPosition);
	outputData.audioData.resize(static_cast<size_t>(chunkSize));
	readChunkData(hFile, static_cast<void*>(outputData.audioData.data()), chunkSize, chunkPosition);

	//バッファの作成
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = static_cast<UINT32>(outputData.audioData.size());	//バッファーのバイト数
	buffer.Flags = XAUDIO2_END_OF_STREAM;									//このバッファの後に他のバッファが来ない
	buffer.pAudioData = outputData.audioData.data();
	outputData.buffer = std::move(buffer);

	//ソースボイスを作成
	outputData.sourceVoices.resize(poolSize);
	for (int i = 0; i < poolSize; i++) {
		HRESULT hr = mXAudio->CreateSourceVoice(&outputData.sourceVoices[i], reinterpret_cast<WAVEFORMATEX*>(&waveFormat));
		if (FAILED(hr)) std::cerr << "ソースボイスの作成に失敗\n";
	}

	return S_OK;
}

HRESULT AudioManager::loadOGGFile(const std::string& filePath, SoundData& outputData, int poolSize)
{
	int channels = 0;
    int sampleRate = 0;
    short* decodedData = nullptr; // 16ビット(2バイト)のPCMデータが入るポインタ

    //Oggファイルを丸ごとPCM(16bit)にデコードする
	//1チャネル当たりのサンプル数
    int sampleCount = stb_vorbis_decode_filename(filePath.c_str(), &channels, &sampleRate, &decodedData);

    if (sampleCount < 0) {
        std::cerr << "Oggのデコードに失敗\n";
        return S_FALSE;
    }

	//2バイトのデータを1バイトの配列に変換
	size_t totalBytes = sampleCount * channels * sizeof(short);
	const BYTE* pByteData = reinterpret_cast<const BYTE*>(decodedData);
	std::vector<BYTE> audioData(pByteData, pByteData + totalBytes);
	outputData.audioData = std::move(audioData);

	//WAVEFORMATEXを作成
    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM; 
	waveFormat.nChannels = channels;
    waveFormat.nSamplesPerSec = sampleRate;
    waveFormat.wBitsPerSample = 16; // stb_vorbis は16ビット(short)でデータを出力
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

    //XAUDIO2_BUFFERを作成
    XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = totalBytes; // 波形データの合計バイト数
    buffer.pAudioData = outputData.audioData.data(); 
	buffer.Flags = XAUDIO2_END_OF_STREAM; // 最後まで再生する
	outputData.buffer = std::move(buffer);

	//ソースボイスを作成
	outputData.sourceVoices.resize(poolSize);
	for (int i = 0; i < poolSize; i++) {
		HRESULT hr = mXAudio->CreateSourceVoice(&outputData.sourceVoices[i], reinterpret_cast<WAVEFORMATEX*>(&waveFormat));
		if (FAILED(hr)) std::cerr << "ソースボイスの作成に失敗\n";
	}


    free(decodedData);

	return S_OK;
}

HRESULT AudioManager::loadMP3File(const std::string& filePath, SoundData& outputData, int poolSize)
{
	mp3dec_t mp3d;
    mp3dec_file_info_t info;

    //MP3ファイルを一括でPCM(16ビット)にデコードする
    //成功すると 0 が返り、info 構造体にデータと各種情報が入る
    if (mp3dec_load(&mp3d, filePath.c_str(), &info, NULL, NULL) != 0) {
        std::cerr << "MP3のデコードに失敗しました。\n";
        return S_FALSE;
    }
	
	//全バイト数宇を計算　※info.samplesはstb_vorbisと違い全サンプル数
    size_t totalBytes = info.samples * sizeof(short); 

    //vectorへコピー
    const BYTE* pByteData = reinterpret_cast<const BYTE*>(info.buffer);
    std::vector<BYTE> audioData(pByteData, pByteData + totalBytes);
	outputData.audioData = std::move(audioData);


    //minimp3 が作った元の生ポインタを解放する
    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = info.channels;
    waveFormat.nSamplesPerSec = info.hz;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

    //XAUDIO2_BUFFERを作成
    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = outputData.audioData.size();
    buffer.pAudioData = outputData.audioData.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;
	outputData.buffer = std::move(buffer);

	//ソースボイスを作成	
	outputData.sourceVoices.resize(poolSize);
	for (int i = 0; i < poolSize; i++) {
		HRESULT hr = mXAudio->CreateSourceVoice(&outputData.sourceVoices[i], reinterpret_cast<WAVEFORMATEX*>(&waveFormat));
		if (FAILED(hr)) std::cerr << "ソースボイスの作成に失敗\n";
	}

    //WAVEFORMATEXを作成
    free(info.buffer);

	return S_OK;

}

