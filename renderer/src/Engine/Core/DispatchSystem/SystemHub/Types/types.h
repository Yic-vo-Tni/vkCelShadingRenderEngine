//
// Created by lenovo on 9/8/2024.
//

#ifndef VKCELSHADINGRENDERER_TYPES_H
#define VKCELSHADINGRENDERER_TYPES_H

#include "Engine/RHI/vkStruct.h"
#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/Descriptor.h"

namespace yic{
    class RenderProcess;
}

namespace et{
    struct FrameEntry;
}

namespace ev {
    template<typename T>
    using opt = std::optional<T>;

//    struct FrameEntry {
//        vk::Image image;
//        vk::ImageView imageView;
//        vk::Semaphore readSemaphore;
//        vk::Semaphore writtenSemaphore;
//    };

    struct pEcs {
        HANA(pEcs,
             (flecs::world * , ecs));
    };

    struct pRenderProcess {
        HANA(pRenderProcess,
             (yic::RenderProcess * , rp));
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
             (GLFWwindow * , window),
             (vk::SwapchainKHR * , swapchain),
             (vk::Extent2D * , currentExtent),
             (std::vector<et::FrameEntry> * , frameEntries),
             (vk::SurfaceFormatKHR * , surfaceFormat),
             (uint32_t * , activeImageIndex),
             (vk::RenderPass * , renderPass),
             (std::vector<vk::Framebuffer> * , framebuffers));
    };

    struct oWindowSizeChange {
        HANA_OPT(oWindowSizeChange,
                 (ImVec2, size),
                 (vk::Extent2D, extent));
    };

    struct eRenderTargetSizeChange{
        ImVec2 size;
    };
    struct eResourcePaths{
        vot::unordered_map<ResFormat, vot::vector<vot::string >> resourcePaths;
    };


}


#endif //VKCELSHADINGRENDERER_TYPES_H
