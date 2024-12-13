#pragma once
/**
 * @file SoundHandle.h
 * @brief SoundHandle
 */
class SoundHandle {
    friend class Audio;
public:
    bool IsValid() const { return index_ != ((size_t)-1); }
private:
    size_t index_ = ((size_t)-1);
};