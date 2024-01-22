#pragma once

#include "../SceneFactory/AbstractSceneFactory/AbstractSceneFactory.h"
#include <string>

class BaseScene;
class CommandContext;
class SceneManager {
public:
	~SceneManager();

	static SceneManager* GetInstance();

	void Initialize(AbstractSceneFactory::Scene scene);

	void Update();

	void Draw(CommandContext& ommandContext);

	void ChangeScene(AbstractSceneFactory::Scene scene);
	void SetSceneFactory(AbstractSceneFactory* abstractSceneFactory) { abstractSceneFactory_ = abstractSceneFactory; }
private:
	BaseScene* scene_ = nullptr;
	BaseScene* nextScene_ = nullptr;
	AbstractSceneFactory* abstractSceneFactory_ = nullptr;
};