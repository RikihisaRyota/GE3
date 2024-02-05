#pragma once

#include <string>

class BaseScene;
class AbstractSceneFactory {
public:
	enum Scene {
		kTitle,
		kGame,

		kCount,
	};

	virtual ~AbstractSceneFactory() = default;

	virtual BaseScene* CreateScene(int scene) = 0;
};