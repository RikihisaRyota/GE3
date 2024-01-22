#pragma once

class CommandContext;
class SceneManager;
class BaseScene {
public:
	virtual ~BaseScene() = default;

	virtual void Initialize() = 0;

	virtual void Finalize() = 0;

	virtual void Update() = 0;

	virtual void Draw(CommandContext& commandContext) = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
private:
	SceneManager* sceneManager_ = nullptr;
};