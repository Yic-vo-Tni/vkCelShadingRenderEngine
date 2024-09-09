//
// Created by lenovo on 8/11/2024.
//

#ifndef VKCELSHADINGRENDERER_RT_H
#define VKCELSHADINGRENDERER_RT_H

#include "RTBuilder.h"
#include "Engine/ECS/Model/ModelStruct.h"

namespace yic {

    class RT {
    public:
        friend struct build;
    protected:
        static vk::Device mDevice;
        static vk::DispatchLoaderDynamic mDispatcher;
    public:
        vkGet auto inst = []{ return Singleton<RT>::get(); };

        struct build{
            auto blas(const vk::AccelerationStructureGeometryKHR& asGeom, const uint32_t& numTri) -> void;
            auto tlas(const std::vector<vkAccel_sptr>& blass, vkAccel_sptr& tlas, bool update = false) -> void;
        };

    };

    inline auto rayTracing = RT::inst();

} // yic

#endif //VKCELSHADINGRENDERER_RT_H
