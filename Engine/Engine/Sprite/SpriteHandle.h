#pragma once

class SpriteHandle {
    friend class SpriteManager;
public:
    operator size_t ()const { return index_; }
    bool IsValid() const { return index_ != ((size_t)-1); }
private:
    size_t index_ = ((size_t)-1);
};