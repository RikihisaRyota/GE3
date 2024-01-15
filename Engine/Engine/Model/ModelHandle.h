#pragma once

class ModelHandle {
    friend class ModelManager;
public:
    operator size_t ()const { return index_; }
    bool IsValid() const { return index_ != ((size_t)-1); }
private:
    size_t index_ = ((size_t)-1);
};