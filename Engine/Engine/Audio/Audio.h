#pragma once
/**
 * @file Audio.h
 * @brief AudioDataの管理
 */
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <xaudio2.h>
#include <wrl.h>

#include <filesystem>

#include <fstream>
#include <vector>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace Engine {

	class Audio {
	public:
		static const int32_t kMaxNumPlayHandles = 1024;

		// チャンクヘッダ
		struct ChunkHearder {
			char id[4]; // チャンク毎のID
			size_t size; // チャンクファイル
		};
		// RIFFヘッダチャンク
		struct RiffHeader {
			ChunkHearder chunk; // "RIFF"
			char type[4]; // "WAVE"
		};
		// FMTチャンク
		struct FormatChunk {
			ChunkHearder chunk; // "fmt"
			WAVEFORMATEX fmt; // 波形フォーマット
		};
		// 音声データ
		struct SoundData {
			std::string filename;
			// 波形フォーマット
			WAVEFORMATEX wfex;
			// バッファの先頭アドレス
			std::vector<BYTE> pBuffer;
			// バッファのサイズ
			size_t bufferSize;
		};
	public:
		static Audio* GetInstance();
	public:
		void Initialize();
		void Update();
		/// <summary>
		/// 音声データ解放
		/// </summary>
		/// <param name="soundData"></param>
		void SoundUnload(int32_t soundHandle);
		/// <summary>
		/// 音声再生
		/// </summary>
		/// <param name="xAudio2"></param>
		/// <param name="soundData"></param>
		int32_t SoundPlayWave(int32_t soundHandle);

		int32_t SoundPlayLoopStart(int32_t soundHandle);
		void SoundPlayLoopEnd(int32_t playHandle);
		/// <summary>
		/// 音声ロード
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
		int32_t SoundLoadWave(const std::string& fileName);
		int32_t SoundLoad(const std::filesystem::path& fileName);

		/// <summary>
		/// 音声を止める
		/// </summary>
		/// <param name="playHandle"></param>
		void StopSound(int32_t playHandle);
		/// <summary>
		/// ピッチの調整
		/// </summary>
		/// <param name="playHandle"></param>
		/// <param name="pitch"></param>
		void SetPitch(int32_t playHandle, float pitch);
		/// <summary>
		/// ボリュームを調整
		/// </summary>
		/// <param name="playHandle"></param>
		/// <param name="volume"></param>
		void SetVolume(int32_t playHandle, float volume);
		bool IsValidPlayHandle(int32_t playHandle);

	private:
		int32_t FindUnusedPlayHandle();

		void DestroyPlayHandle(int32_t playHandle);

		Audio() = default;
		Audio(const Audio&) = delete;
		Audio& operator=(const Audio&) = delete;
		~Audio();

		Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
		IXAudio2MasteringVoice* masterVoice_ = nullptr;
		std::vector<SoundData> soundData_;
		IXAudio2SourceVoice* sourceVoices_[kMaxNumPlayHandles]{ nullptr };
	};
}