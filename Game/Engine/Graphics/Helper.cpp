#include "Helper.h"
namespace  {
    D3D12_RASTERIZER_DESC CreateRasterizerDesc(
        D3D12_FILL_MODE fillMode,
        D3D12_CULL_MODE cullMode,
        BOOL frontCounterClockwise = FALSE,
        INT depthBias = D3D12_DEFAULT_DEPTH_BIAS,
        FLOAT depthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
        FLOAT slopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        BOOL depthClipEnable = TRUE,
        BOOL multisampleEnable = FALSE,
        BOOL antialiasedLineEnable = FALSE,
        UINT forcedSampleCount = 0,
        D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF) {
        D3D12_RASTERIZER_DESC rasterizerDesc{};
        rasterizerDesc.FillMode = fillMode;
        rasterizerDesc.CullMode = cullMode;
        rasterizerDesc.FrontCounterClockwise = frontCounterClockwise;
        rasterizerDesc.DepthBias = depthBias;
        rasterizerDesc.DepthBiasClamp = depthBiasClamp;
        rasterizerDesc.SlopeScaledDepthBias = slopeScaledDepthBias;
        rasterizerDesc.DepthClipEnable = depthClipEnable;
        rasterizerDesc.MultisampleEnable = multisampleEnable;
        rasterizerDesc.AntialiasedLineEnable = antialiasedLineEnable;
        rasterizerDesc.ForcedSampleCount = forcedSampleCount;
        rasterizerDesc.ConservativeRaster = conservativeRaster;
        return rasterizerDesc;
    }

    D3D12_BLEND_DESC CreateBlendDesc(BOOL blendEnabled,
        D3D12_BLEND srcBlend, D3D12_BLEND destBlend, D3D12_BLEND_OP blendOp,
        D3D12_BLEND srcBlendAlpha, D3D12_BLEND destBlendAlpha, D3D12_BLEND_OP blendOpAlpha,
        UINT8 renderTargetWriteMask) {
        D3D12_BLEND_DESC blendDesc{};
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = blendEnabled;
        blendDesc.RenderTarget[0].SrcBlend = srcBlend;
        blendDesc.RenderTarget[0].DestBlend = destBlend;
        blendDesc.RenderTarget[0].BlendOp = blendOp;
        blendDesc.RenderTarget[0].SrcBlendAlpha = srcBlendAlpha;
        blendDesc.RenderTarget[0].DestBlendAlpha = destBlendAlpha;
        blendDesc.RenderTarget[0].BlendOpAlpha = blendOpAlpha;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = renderTargetWriteMask;
        return blendDesc;
    }

    D3D12_DEPTH_STENCIL_DESC CreateDepthState(BOOL depthEnabled, D3D12_DEPTH_WRITE_MASK depthWriteMask, D3D12_COMPARISON_FUNC depthFunc) {
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = depthEnabled;
        depthStencilDesc.DepthWriteMask = depthWriteMask;
        depthStencilDesc.DepthFunc = depthFunc;
        return depthStencilDesc;
    }
}

namespace Helper {
    const D3D12_RASTERIZER_DESC RasterizerDefault = CreateRasterizerDesc(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK);
    const D3D12_RASTERIZER_DESC RasterizerNoCull = CreateRasterizerDesc(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);
    const D3D12_RASTERIZER_DESC RasterizerWireframe = CreateRasterizerDesc(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE);

    const D3D12_BLEND_DESC BlendNoColorWrite =
        CreateBlendDesc(FALSE,
            D3D12_BLEND_SRC_ALPHA,
            D3D12_BLEND_INV_SRC_ALPHA,
            D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE,
            D3D12_BLEND_INV_SRC_ALPHA,
            D3D12_BLEND_OP_ADD,
            D3D12_COLOR_WRITE_ENABLE_ALL); // カラーチャンネルへの書き込みなし

    const D3D12_BLEND_DESC BlendDisable =
        CreateBlendDesc(FALSE,
            D3D12_BLEND_SRC_ALPHA,
            D3D12_BLEND_INV_SRC_ALPHA,
            D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE,
            D3D12_BLEND_ZERO,
            D3D12_BLEND_OP_ADD,
            0); // ブレンド無効

    const D3D12_BLEND_DESC BlendAlpha =
        CreateBlendDesc(TRUE,
            D3D12_BLEND_SRC_ALPHA,
            D3D12_BLEND_INV_SRC_ALPHA,
            D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE,
            D3D12_BLEND_ZERO,
            D3D12_BLEND_OP_ADD,
            D3D12_COLOR_WRITE_ENABLE_ALL); // アルファブレンディング

    const D3D12_BLEND_DESC BlendMultiply =
        CreateBlendDesc(TRUE,
            D3D12_BLEND_DEST_COLOR,
            D3D12_BLEND_ZERO,
            D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ZERO,
            D3D12_BLEND_SRC_COLOR,
            D3D12_BLEND_OP_ADD,
            D3D12_COLOR_WRITE_ENABLE_ALL); // 乗算合成

    const D3D12_BLEND_DESC BlendAdditive =
        CreateBlendDesc(TRUE,
            D3D12_BLEND_SRC_ALPHA,
            D3D12_BLEND_ONE,
            D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE,
            D3D12_BLEND_ZERO,
            D3D12_BLEND_OP_ADD,
            D3D12_COLOR_WRITE_ENABLE_ALL); // 加算合成

    const D3D12_BLEND_DESC BlendSubtract =
        CreateBlendDesc(TRUE,
            D3D12_BLEND_SRC_ALPHA,
            D3D12_BLEND_ONE,
            D3D12_BLEND_OP_REV_SUBTRACT,
            D3D12_BLEND_ONE,
            D3D12_BLEND_ZERO,
            D3D12_BLEND_OP_ADD,
            D3D12_COLOR_WRITE_ENABLE_ALL); // 減算合成


    const D3D12_DEPTH_STENCIL_DESC DepthStateDisabled = CreateDepthState(FALSE, D3D12_DEPTH_WRITE_MASK_ZERO, D3D12_COMPARISON_FUNC_ALWAYS);
    const D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite = CreateDepthState(TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS_EQUAL);
}