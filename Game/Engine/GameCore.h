#pragma once

#include "Graphics/RenderManager.h"
#include "WinApp/WinApp.h"
#include "Input/Input.h"

namespace GameCore {
	void Initialize();
	bool BeginFrame();
	void Shutdown();
}
