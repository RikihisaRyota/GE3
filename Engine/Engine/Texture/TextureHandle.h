#pragma once

class TextureHandle {
    friend class TextureManager;
public:
    bool IsValid() const { return index_ != ((size_t)-1); }
    bool operator==(const TextureHandle& other) const {
        return index_ == other.index_;
    }

    // 代入演算子
    void operator=(const TextureHandle& other) {
        index_ = other.index_;
    }

    // 暗黙のuint32_tへの変換関数
    operator uint32_t() const {
        return static_cast<uint32_t>(index_);
    }

private:
    size_t index_ = ((size_t)-1);
};
