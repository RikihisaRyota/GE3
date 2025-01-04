#pragma once

#include <string>
#include <memory>
#include <vector>

#include "../SceneFactory/AbstractSceneFactory/AbstractSceneFactory.h"
#include "../Transition/Transition.h"

#include "Engine/Math/ViewProjection.h"

class BaseScene;
class CommandContext;
class SceneManager {
public:
	~SceneManager();

	static SceneManager* GetInstance();

	void Initialize(int scene, ViewProjection* viewProjection);

	void Update(CommandContext& commandContext,ViewProjection* viewProjection);

	void Draw(CommandContext& ommandContext);

	void Finalize();
	// シーンチェンジ
	void ChangeScene(int scene);
	void SetSceneFactory(AbstractSceneFactory* abstractSceneFactory) { abstractSceneFactory_ = abstractSceneFactory; }
private:
	BaseScene* scene_ = nullptr;
	BaseScene* nextScene_ = nullptr;
	AbstractSceneFactory* abstractSceneFactory_ = nullptr;
	std::unique_ptr<Transition> transition_;
	int currentScene_;
	std::vector<const char*> sceneNames_;
};