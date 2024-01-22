#pragma once

#include <xaudio2.h>
#include <wrl.h>

#include <fstream>
#include <vector>


class Audio {
public:
    static const int32_t kMaxNumPlayHandles = 1024;

    // チャンクヘッダ
    struct ChunkHearder {
        char id[4]; // チャンク毎のID
        int32_t size; // チャンクファイル
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
        uint32_t bufferSize;
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

    void StopSound(int32_t playHandle);
    void SetPitch(int32_t playHandle, float pitch);
    void SetValume(int32_t playHandle, float volume);
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

