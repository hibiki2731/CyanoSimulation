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

	void playBGM(std::string soundID);
	void playSE(std::string soundID);
private:
	struct SoundData {
		std::vector<BYTE> audioData;
		WAVEFORMATEXTENSIBLE waveFormat;
	};

	struct SoundInstance {
		IXAudio2SourceVoice* mSourceVoice = nullptr;
		bool isLoop = false;

		~SoundInstance() {
			if (mSourceVoice) {
				mSourceVoice->Stop();
				mSourceVoice->DestroyVoice();
		        mSourceVoice = nullptr;
		    }
		}
	};

	void initXAudio();
	void loadSoundFiles();
	void stopBGM();
	void stopAllSounds();
	void finishBGM();
	void finishAllSounds();
	void clearFinishedSounds();
	HRESULT findChunk(HANDLE hFile, DWORD targetFourcc, DWORD& chunkSize, DWORD& chunkDataPosition); //FOURCCからそのチャンクのデータサイズとデータの位置を探す。
	HRESULT readChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset);
	HRESULT loadWAVFile(const std::string& filePath, SoundData& outputData);

	DWORD fourccRIFF;
	DWORD fourccFMT;
	DWORD fourccDATA;
	DWORD fourccWAVE;

	ComPtr<IXAudio2> mXAudio;
	IXAudio2MasteringVoice* mMasteringVoice;
	std::vector<std::unique_ptr<SoundInstance>> mSoundInstances;
	std::unordered_map<std::string, SoundData> mSoundDataList;
	SoundInstance* mCurrentBGM;

};

