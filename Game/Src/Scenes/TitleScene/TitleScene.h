#pragma once

#include "Src/Scenes/BaseScene/BaseScene.h"

class CommandContext;
class TitleScene : public BaseScene {
public:
	TitleScene();
	~TitleScene();
	void Initialize() override;
	void Update() override;
	void Draw(CommandContext& commandContext) override;
	void Finalize() override;
private:

};
