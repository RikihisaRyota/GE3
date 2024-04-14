// ModelHandle.h

#pragma once

class ModelHandle {
	friend class ModelManager;
public:
	static const ModelHandle kMaxModeHandle;

	// デフォルトのコンストラクタを追加
	ModelHandle() = default;
	// size_t からの暗黙の型変換を行う関数を追加
	ModelHandle(size_t value) : index_(value) {}

	operator size_t () const { return index_; }
	bool IsValid() const { return index_ != ((size_t)-1); }
private:
	size_t index_ = ((size_t)-1);
};
