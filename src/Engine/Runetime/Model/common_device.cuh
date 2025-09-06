//
// Created by lenovo on 10/14/2024.
//

#ifndef VKCELSHADINGRENDERER_COMMON_DEVICE_CUH
#define VKCELSHADINGRENDERER_COMMON_DEVICE_CUH

#include "device_launch_parameters.h"

namespace gpu{
    void transformVerticesCUDA(float3* vertices, float3 center, uint32_t numVertices);
}


#endif //VKCELSHADINGRENDERER_COMMON_DEVICE_CUH
