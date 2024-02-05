#pragma once
#include <d3d12.h>
#include <Windows.h>

#ifdef ENABLE_IMGUI
#include "../Imgui/imgui.h"
#endif // ENABLE_IMGUI

class CommandContext;

class ImGuiManager {
public:
    static ImGuiManager* GetInstance();

    void Initialize(HWND hWnd, DXGI_FORMAT rtvFormat);
    void NewFrame();
    void Render(CommandContext& commandContext);
    void Shutdown();

private:
    ImGuiManager() = default;
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

};

