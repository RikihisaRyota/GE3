#pragma once

#include <d3d12.h>

namespace Helper {
    extern const D3D12_RASTERIZER_DESC RasterizerDefault;
    extern const D3D12_RASTERIZER_DESC RasterizerNoCull;
    extern const D3D12_RASTERIZER_DESC RasterizerWireframe;

    extern const D3D12_BLEND_DESC BlendNoColorWrite;
    extern const D3D12_BLEND_DESC BlendDisable;     // ブレンド無効
    extern const D3D12_BLEND_DESC BlendAlpha;       // アルファブレンド
    extern const D3D12_BLEND_DESC BlendMultiply;    // 乗算合成
    extern const D3D12_BLEND_DESC BlendAdditive;    // 加算合成
    extern const D3D12_BLEND_DESC BlendSubtract;    // 加算合成

    extern const D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
    extern const D3D12_DEPTH_STENCIL_DESC DepthStateRead;
    extern const D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
}