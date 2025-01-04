#pragma once

#include "Engine/Math/ViewProjection.h"

class CommandContext;
class SceneManager;
class BaseScene {
public:
	virtual ~BaseScene() = default;
	// 初期化
	virtual void Initialize() = 0;
	// Finalize
	virtual void Finalize() = 0;
	// 更新
	virtual void Update(CommandContext& commandContext) = 0;
	// 描画
	virtual void Draw(CommandContext& commandContext) = 0;
	// Setter
	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
	virtual void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
protected:
	SceneManager* sceneManager_ = nullptr;
	ViewProjection* viewProjection_ = nullptr;
};