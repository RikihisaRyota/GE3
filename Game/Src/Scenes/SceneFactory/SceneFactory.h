#pragma once

#include "AbstractSceneFactory/AbstractSceneFactory.h"

class SceneFactory : public AbstractSceneFactory {
public:
	BaseScene* CreateScene(Scene scene) override;
};