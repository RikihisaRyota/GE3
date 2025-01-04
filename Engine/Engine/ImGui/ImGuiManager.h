#pragma once
/**
 * @file ImGuiManager.h
 * @brief ImGuiの管理
 */
#include <d3d12.h>
#include <Windows.h>

#ifdef ENABLE_IMGUI
#include "../Imgui/imgui.h"
#endif // ENABLE_IMGUI

class CommandContext;

class ImGuiManager {
public:
    static ImGuiManager* GetInstance();
    // 初期化
    void Initialize(HWND hWnd, DXGI_FORMAT rtvFormat);
    // NewFrame
    void NewFrame();
    // 描画
    void Render(CommandContext& commandContext);
    // Shutdown
    void Shutdown();

private:
    ImGuiManager() = default;
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

};

