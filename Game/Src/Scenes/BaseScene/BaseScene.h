#pragma once

#include "Engine/Math/ViewProjection.h"

class CommandContext;
class SceneManager;
class BaseScene {
public:
	virtual ~BaseScene() = default;

	virtual void Initialize() = 0;

	virtual void Finalize() = 0;

	virtual void Update(CommandContext& commandContext) = 0;

	virtual void Draw(CommandContext& commandContext) = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
	virtual void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
protected:
	SceneManager* sceneManager_ = nullptr;
	ViewProjection* viewProjection_ = nullptr;
};