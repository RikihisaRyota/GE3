#pragma once

#include "Graphics/RenderManager.h"
#include "WinApp/WinApp.h"
#include "Input/Input.h"
#include "ShderCompiler/ShaderCompiler.h"
#include "../GameScene.h"

namespace GameCore {
	void Initialize();
	bool BeginFrame();
	void Shutdown();
}
