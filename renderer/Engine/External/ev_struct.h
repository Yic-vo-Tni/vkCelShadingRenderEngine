//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_EV_STRUCT_H
#define VKCELSHADINGRENDERER_EV_STRUCT_H

#define HANA(...) BOOST_HANA_DEFINE_STRUCT(__VA_ARGS__)

#include "rhi_struct.h"
#include "rs_struct.h"

namespace ev {

    struct pWindowContext {
        HANA(pWindowContext,
             (GLFWwindow * , window),
             (vk::Extent2D*, extent));
    };

    struct pVkSetupContext {
        HANA(pVkSetupContext,
             (vk::Instance * , instance),
             (vk::DispatchLoaderDynamic * , dynamicDispatcher),
             (vk::DebugUtilsMessengerEXT * , debugMessenger),
             (vk::PhysicalDevice * , physicalDevice),
             (vk::Device * , device));
    };


    struct oWindowSizeChange{
        HANA(oWindowSizeChange,
             (std::optional<vk::Extent2D>, extent));
    };

    struct pVkRenderContext {
        HANA(pVkRenderContext,
             (vk::SwapchainKHR * , swapchain),
             (vk::Extent2D * , currentExtent),
             (vot::vector<vot::FrameEntry> * , frameEntries),
             (vk::SurfaceFormatKHR * , surfaceFormat),
             (uint32_t * , activeImageIndex),
             (vk::CommandBuffer * , cmd),
             (vk::RenderPass * , renderPass),
             (vot::vector<vk::Framebuffer> * , framebuffers));
    };

    struct freeCameraController{
        HANA(freeCameraController,
             (std::optional<bool>, W),
             (std::optional<bool>, A),
             (std::optional<bool>, S),
             (std::optional<bool>, D),
             (std::optional<bool>, cursor),
             (std::optional<bool>, scroll),
             (std::optional<bool>, firstM),
             (std::optional<double>, xPos),
             (std::optional<double>, yPos),
             (std::optional<double>, xOffset),
             (std::optional<double>, yOffset));
    };


    struct glKeyInput {
        int key;
        int action;
        int scancode;
        int mods;
    };

    struct glMouseInput {
        int button;
        int action;
        int mods;
    };

    struct glCursorPosInput {
        double xpos;
        double ypos;
    };

    struct glScrollInput {
        double xoffset;
        double yoffset;
    };


//    struct sModelPaths{ vot::vector<vot::string> paths; };
//    struct sImagePaths{ vot::vector<vot::string> paths; };
    struct tResourcesPaths{ vot::vector<vot::string> paths; };

    struct tModelLoaded{};

    struct tUpdateScene{};
}


#endif //VKCELSHADINGRENDERER_EV_STRUCT_H
