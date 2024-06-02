//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTTYPES_H
#define VKCELSHADINGRENDERER_EVENTTYPES_H

#include "Engine/RHI/vkStruct.h"

#define struct_y(...) BOOST_HANA_DEFINE_STRUCT(__VA_ARGS__)

namespace EventTypes {
    
    template<typename T>
    using opt = std::optional<T>;

    struct WindowContext {
        struct_y(WindowContext,
                 (opt<int>, w),
                 (opt<int>, h),
                 (std::shared_ptr<GLFWwindow>, window));
    };

    struct KeyInput {
        struct_y(KeyInput,
                 (opt<int>, key),
                 (opt<int>, action),
                 (opt<int>, scancode),
                 (opt<int>, mods));
    };

    struct vkInitContext {
        struct_y(vkInitContext,
                 (vk::Instance, instance),
                 (std::shared_ptr<vk::DispatchLoaderDynamic>, dynamicDispatcher),
                 (vk::DebugUtilsMessengerEXT, debugMessenger),
                 (vk::SurfaceKHR, surface));
    };

    struct vkDeviceContext {
        struct_y(vkDeviceContext,
                 (vk::PhysicalDevice, physicalDevice),
                 (vk::Device, device),
                 (std::shared_ptr<yic::QueueFamily>, queueFamily));
    };


}

#endif //VKCELSHADINGRENDERER_EVENTTYPES_H
