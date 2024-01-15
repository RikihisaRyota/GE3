#pragma once

class TextureHandle {
    friend class TextureManager;
public:
    bool IsValid() const { return index_ != ((size_t)-1); }
private:
    size_t index_ = ((size_t)-1);
};