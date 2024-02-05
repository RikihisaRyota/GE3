#include "SceneFactory.h"

#include "../BaseScene/BaseScene.h"
#include "../GameScene/GameScene.h"
#include "../TitleScene/TitleScene.h"

BaseScene* SceneFactory::CreateScene(int scene) {
    BaseScene* newScene = nullptr;
    switch (scene) {
    case AbstractSceneFactory::kTitle:
        newScene = new TitleScene();
        break;
    case AbstractSceneFactory::kGame:
        newScene = new GameScene();
        break;
    case AbstractSceneFactory::kCount:
        break;
    default:
        break;
    }
    return newScene;
}
