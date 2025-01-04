#pragma once

#include <vector>

#include "Engine/Sprite/SpriteHandle.h"

class CommandContext;
class PlayerUI {
public:
	enum  {
		kReticle,
		kCount,
	};
	// コンストラクタ
	PlayerUI();
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw(CommandContext& commandContext);

	const SpriteHandle& GetHandle(uint32_t index)const { return handle_.at(index); }
private:
	std::vector<SpriteHandle>handle_;

};