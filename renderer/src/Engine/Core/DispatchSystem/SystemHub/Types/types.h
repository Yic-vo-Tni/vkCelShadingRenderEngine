//
// Created by lenovo on 9/8/2024.
//

#ifndef VKCELSHADINGRENDERER_TYPES_H
#define VKCELSHADINGRENDERER_TYPES_H

#include "Engine/RHI/vkStruct.h"
#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/Descriptor.h"

namespace yic{
    class RenderPrcess;
}

namespace ev {
    template<typename T>
    using opt = std::optional<T>;

    struct FrameEntry {
        vk::Image image;
        vk::ImageView imageView;
        vk::Semaphore readSemaphore;
        vk::Semaphore writtenSemaphore;
    };

    struct pEcs {
        HANA(pEcs,
             (flecs::world * , ecs));
    };

    struct pRenderProcess {
        HANA(pRenderProcess,
             (yic::RenderPrcess * , rp));
    };

    struct pVkSetupContext {
        HANA(pVkSetupContext,
             (vk::Instance * , instance),
             (vk::DispatchLoaderDynamic * , dynamicDispatcher),
             (vk::DebugUtilsMessengerEXT * , debugMessenger),
             (vk::PhysicalDevice * , physicalDevice),
             (vk::Device * , device),
             (yic::QueueFamily * , queueFamily));
    };

    struct hVkRenderContext {
        HANA(hVkRenderContext,
             (opt<ImVec2>, size),
             (vk::Extent2D * , extent),
             (GLFWwindow * , window),
             (vk::SwapchainKHR, swapchain),
             (vk::Extent2D * , currentExtent),
             (vot::vector<FrameEntry> * , fameEntries),
             (uint8_t * , activeImgIndex),
             (std::vector<vk::Framebuffer> * , framebuffers));
    };

}


#endif //VKCELSHADINGRENDERER_TYPES_H
