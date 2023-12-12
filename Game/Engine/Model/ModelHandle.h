#pragma once

class ModelHandle {
    friend class ModelManager;
public:
    bool IsValid() const { return index_ != ((size_t)-1); }
private:
    size_t index_ = ((size_t)-1);
};