#include "Audio.h"

#include <cassert>

#pragma comment(lib,"xaudio2.lib")

namespace Engine {

	Audio* Audio::GetInstance() {
		static Audio instans;
		return &instans;
	}

	Audio::~Audio() {
		for (int32_t i = 0; i < kMaxNumPlayHandles; ++i) {
			DestroyPlayHandle(i);
		}

		if (masterVoice_) {
			masterVoice_->DestroyVoice();
			masterVoice_ = nullptr;
		}
	}

	void Audio::Initialize() {
		HRESULT result;

		result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		assert(SUCCEEDED(result));

		result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
		assert(SUCCEEDED(result));

		// XAudioエンジンのインスタンスを作成
		result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
		assert(SUCCEEDED(result));
		// マスターボイスを作成
		result = xAudio2_->CreateMasteringVoice(&masterVoice_);
		assert(SUCCEEDED(result));

		for (int32_t i = 0; i < kMaxNumPlayHandles; ++i) {
			DestroyPlayHandle(i);
		}
	}

	void Audio::Update() {
		for (int32_t i = 0; i < kMaxNumPlayHandles; ++i) {
			if (sourceVoices_[i]) {
				XAUDIO2_VOICE_STATE state{ };
				sourceVoices_[i]->GetState(&state);
				if (state.BuffersQueued == 0) {
					DestroyPlayHandle(i);
				}
			}
		}
	}

	int32_t Audio::SoundPlayWave(int32_t soundHandle) {
		HRESULT result;
		const SoundData& soundData = soundData_.at(soundHandle);

		// 再生する波形データの設定
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = soundData.pBuffer.data();
		buf.AudioBytes = UINT32(soundData.bufferSize);
		buf.Flags = XAUDIO2_END_OF_STREAM;

		int32_t playHandle = FindUnusedPlayHandle();
		// プレイハンドルがいっぱい
		assert(playHandle < kMaxNumPlayHandles);

		// SourceVoice の作成
		IXAudio2SourceVoice* pSourceVoice = nullptr;
		result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
		assert(SUCCEEDED(result));

		result = pSourceVoice->SubmitSourceBuffer(&buf);
		assert(SUCCEEDED(result));

		result = pSourceVoice->Start();
		assert(SUCCEEDED(result));

		sourceVoices_[playHandle] = pSourceVoice;
		return playHandle;
	}

	int32_t Audio::SoundPlayLoopStart(int32_t soundHandle) {
		HRESULT result;
		const SoundData& soundData = soundData_.at(soundHandle);

		// 再生する波形データの設定
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = soundData.pBuffer.data();
		buf.AudioBytes = UINT32(soundData.bufferSize);
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;

		int32_t playHandle = FindUnusedPlayHandle();
		// プレイハンドルがいっぱい
		assert(playHandle < kMaxNumPlayHandles);

		// SourceVoice の作成
		IXAudio2SourceVoice* pSourceVoice = nullptr;
		result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
		assert(SUCCEEDED(result));

		result = pSourceVoice->SubmitSourceBuffer(&buf);
		assert(SUCCEEDED(result));

		result = pSourceVoice->Start();
		assert(SUCCEEDED(result));

		sourceVoices_[playHandle] = pSourceVoice;
		return playHandle;
	}

	void Audio::SoundPlayLoopEnd(int32_t playHandle) {
		// soundHandle に対応する SourceVoice を取得
		if (IsValidPlayHandle(playHandle)) {
			sourceVoices_[playHandle]->Stop();
			DestroyPlayHandle(playHandle);
		}
	}

	int32_t Audio::SoundLoadWave(const std::string& fileName) {

		auto iter = std::find_if(soundData_.begin(), soundData_.end(), [&](const SoundData& soundData) {
			return soundData.filename == fileName;
			});
		if (iter != soundData_.end()) {
			return int32_t(std::distance(soundData_.begin(), iter));
		}

#pragma region ファイルオープン
		// ファイル入出ストリームのインスタンス
		std::ifstream file;
		// wavファイルをバイナリモードで開く
		file.open("Resources/Audios/" + fileName, std::ios_base::binary, std::ios_base::binary);
		// ファイルオープン失敗を検出する
		assert(file.is_open());
#pragma endregion


#pragma region wavデータ読み込み
		// RIFFヘッターの読み込み
		RiffHeader riff;
		file.read((char*)&riff, sizeof(riff));
		// ファイルがRIFFがチェック
		if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
			assert(0);
		}
		// タイプがWAVEがチェック
		if (strncmp(riff.type, "WAVE", 4) != 0) {
			assert(0);
		}

		auto SearchChunk = [&file](const char* id, ChunkHearder& dest) {
			ChunkHearder data{};
			const uint32_t kMaxSearchCount = 100;
			for (uint32_t i = 0; i < kMaxSearchCount;) {
				file.read((char*)&data, sizeof(ChunkHearder));
				// 目的のチャンクだったら終了
				if (strncmp(data.id, id, 4) == 0) {
					dest = data;
					return true;
				}
				// 余計なチャンクを飛ばす
				file.seekg(data.size, std::ios_base::cur);
			}
			return false;
			};

		FormatChunk formatChunk{};
		if (!SearchChunk("fmt ", formatChunk.chunk)) {
			assert(false);
		}
		// チャンク本体の読み込み
		std::vector<char> formatData(formatChunk.chunk.size);
		file.read(formatData.data(), formatChunk.chunk.size);
		memcpy(&formatChunk.fmt, formatData.data(), sizeof(formatChunk.fmt));

		ChunkHearder dataChunk{};
		if (!SearchChunk("data", dataChunk)) {
			assert(false);
		}
		// Dataチャンクのデータ部（波形データ）の読み込み
		std::vector<BYTE> pBuffer(dataChunk.size);
		file.read(reinterpret_cast<char*>(pBuffer.data()), dataChunk.size);

		// Waveファイルを閉じる
		file.close();
#pragma endregion
#pragma region 読み込んだ音声データのreturn
		// returnする為の音声データ
		SoundData soundData = {};
		soundData.filename = fileName;
		soundData.wfex = formatChunk.fmt;
		soundData.pBuffer = std::move(pBuffer);
		soundData.bufferSize = dataChunk.size;
#pragma endregion
		soundData_.emplace_back(soundData);

		return int32_t(soundData_.size() - 1);
	}

	int32_t Audio::SoundLoad(const std::filesystem::path& fileName) {
		IMFSourceReader* pMFSourceReader{ nullptr };
		MFCreateSourceReaderFromURL(fileName.c_str(), NULL, &pMFSourceReader);
		IMFMediaType* pMFMediaType{ nullptr };
		MFCreateMediaType(&pMFMediaType);
		pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
		pMFSourceReader->SetCurrentMediaType(DWORD(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType);

		pMFMediaType->Release();
		pMFMediaType = nullptr;
		pMFSourceReader->GetCurrentMediaType(DWORD(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &pMFMediaType);

		SoundData soundData = {};
		soundData.filename = fileName.string();

		WAVEFORMATEX* pWaveFormatEx = nullptr;
		MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &pWaveFormatEx, nullptr);
		soundData.wfex = *pWaveFormatEx;
		CoTaskMemFree(pWaveFormatEx);

		std::vector<BYTE> mediaData;
		while (true) {
			IMFSample* pMFSample{ nullptr };
			DWORD dwStreamFlags{ 0 };
			pMFSourceReader->ReadSample(DWORD(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

			if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
				break;
			}

			IMFMediaBuffer* pMFMediaBuffer{ nullptr };
			pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

			BYTE* pBuffer{ nullptr };
			DWORD cbCurrentLength{ 0 };
			pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

			mediaData.resize(mediaData.size() + cbCurrentLength);
			memcpy(mediaData.data() + mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

			pMFMediaBuffer->Unlock();

			pMFMediaBuffer->Release();
			pMFSample->Release();
		}
		soundData.bufferSize = mediaData.size();
		soundData.pBuffer = std::move(mediaData);

		soundData_.emplace_back(soundData);

		return int32_t(soundData_.size() - 1);
	}

	void Audio::StopSound(int32_t playHandle) {
		assert(playHandle < kMaxNumPlayHandles);
		DestroyPlayHandle(playHandle);
	}

	void Audio::SetPitch(int32_t playHandle, float pitch) {
		assert(playHandle < kMaxNumPlayHandles);
		sourceVoices_[playHandle]->SetFrequencyRatio(pitch);
	}

	void Audio::SetVolume(int32_t playHandle, float volume) {
		assert(playHandle < kMaxNumPlayHandles);
		sourceVoices_[playHandle]->SetVolume(volume);
	}

	bool Audio::IsValidPlayHandle(int32_t playHandle) {
		return playHandle < kMaxNumPlayHandles && sourceVoices_[playHandle] != nullptr;
	}


	int32_t Audio::FindUnusedPlayHandle() {
		for (int32_t i = 0; i < kMaxNumPlayHandles; ++i) {
			if (sourceVoices_[i] == nullptr) {
				return i;
			}
		}
		return int32_t(-1);
	}

	void Audio::DestroyPlayHandle(int32_t playHandle) {
		assert(playHandle < kMaxNumPlayHandles);
		if (sourceVoices_[playHandle]) {
			sourceVoices_[playHandle]->DestroyVoice();
			sourceVoices_[playHandle] = nullptr;
		}
	}


	void Audio::SoundUnload(int32_t soundHandle) {
		// バッファのメモリを解放
		soundData_.at(soundHandle).pBuffer.clear();
		soundData_.at(soundHandle).bufferSize = 0;
		soundData_.at(soundHandle).wfex = {};
		soundData_.erase(soundData_.begin() + soundHandle);
	}
}