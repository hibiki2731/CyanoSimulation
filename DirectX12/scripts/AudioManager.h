#pragma once
#pragma comment(lib, "xaudio2.lib")
#include <xaudio2.h>
#include <wrl/client.h>
#include <vector>
#include <unordered_map>

using Microsoft::WRL::ComPtr;

class AudioManager
{
public:
	AudioManager();
	~AudioManager();

	void playBGM(const std::string& soundID);
	IXAudio2SourceVoice* playSE(const std::string& soundID);
	void pauseBGM();
	void pauseAllSounds();

private:

	struct SoundData {
		std::vector<IXAudio2SourceVoice*> sourceVoices;
		std::vector<BYTE> audioData;
		XAUDIO2_BUFFER buffer;

	public:
		~SoundData() {
			for (auto voice : sourceVoices) {
				if (voice) {
					voice->Stop(0);
					voice->FlushSourceBuffers();
					voice->DestroyVoice();
					voice = nullptr;
				}
			}
		};
	};

	void initXAudio();
	void loadSoundFiles();
	void finishBGM();
	void finishAllSounds();
	void clearFinishedSounds();
	HRESULT findChunk(HANDLE hFile, DWORD targetFourcc, DWORD& chunkSize, DWORD& chunkDataPosition); //FOURCCからそのチャンクのデータサイズとデータの位置を探す。
	HRESULT readChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset);
	HRESULT loadWAVFile(const std::string& filePath, SoundData& outputData, int poolSize = 1);
	HRESULT loadOGGFile(const std::string& filePath, SoundData& outputData, int poolSize = 1);
	HRESULT loadMP3File(const std::string& filePath, SoundData& outputData, int poolSize = 1);

	DWORD fourccRIFF;
	DWORD fourccFMT;
	DWORD fourccDATA;
	DWORD fourccWAVE;

	ComPtr<IXAudio2> mXAudio;
	IXAudio2MasteringVoice* mMasteringVoice;
	std::vector<IXAudio2SourceVoice*> mNowPlayingVoicess;
	std::unordered_map<std::string, std::unique_ptr<SoundData>> mSoundDataList;
	IXAudio2SourceVoice* mCurrentBGM;

};

