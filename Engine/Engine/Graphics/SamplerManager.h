#pragma once

#include "DescriptorHandle.h"

namespace SamplerManager {
    extern DescriptorHandle LinearWrap;
    extern DescriptorHandle LinearClamp;
    extern DescriptorHandle LinearBorder;

    extern DescriptorHandle Anisotropic;

    extern DescriptorHandle PointWrap;
    extern DescriptorHandle PointClamp;
    extern DescriptorHandle PointBorder;

    void Initialize();
}