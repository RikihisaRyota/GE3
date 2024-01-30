#pragma once

class TextureHandle {
    friend class TextureManager;
public:
    bool IsValid() const { return index_ != ((size_t)-1); }
    bool operator==(const TextureHandle& other) const {
        return index_ == other.index_;
    }
private:
    size_t index_ = ((size_t)-1);
};