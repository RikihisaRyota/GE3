#pragma once

#include <memory>
#include <list>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

class BossModel {

};

class CommandContext;
class BossModelManager {
public:
	BossModelManager();
	void Initialize();
	void Update();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
private:

};