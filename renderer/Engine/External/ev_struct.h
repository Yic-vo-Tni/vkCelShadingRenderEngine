//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_EV_STRUCT_H
#define VKCELSHADINGRENDERER_EV_STRUCT_H

#include "rhi_struct.h"
#include "rs_struct.h"

namespace ev {

    struct pWindowContext {
        GLFWwindow *window = nullptr;
        vk::Extent2D *extent = nullptr;
    };

    struct pVkSetupContext {
        vk::Instance *instance = nullptr;
        vk::detail::DispatchLoaderDynamic *dynamicDispatcher = nullptr;
        vk::DebugUtilsMessengerEXT *debugMessenger = nullptr;
        vk::PhysicalDevice *physicalDevice = nullptr;
        vk::Device *device = nullptr;
    };

    struct oWindowSizeChange {
        std::optional<vk::Extent2D> extent{};
    };

    struct pVkRenderContext {
        vk::SwapchainKHR *swapchain = nullptr;
        vk::Extent2D *currentExtent = nullptr;
        vot::vector<vot::FrameEntry> *frameEntries = nullptr;
        vk::SurfaceFormatKHR *surfaceFormat = nullptr;
        uint32_t *activeImageIndex = nullptr;
        vk::CommandBuffer *cmd = nullptr;
        vk::RenderPass *renderPass = nullptr;
        vot::vector<vk::Framebuffer> *framebuffers = nullptr;
    };

    struct vFreeCameraController {
        bool W = false;
        bool A = false;
        bool S = false;
        bool D = false;
        bool cursor = false;
        bool scroll = false;
        bool firstM = false;

        double xPos = 0.0;
        double yPos = 0.0;
        double xOffset = 0.0;
        double yOffset = 0.0;
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

    struct tResourcesPaths{ vot::vector<vot::string> paths; };

    struct tModelLoaded {
        vot::BasicInfoComponent bic{};
        vot::VertexDataComponent vdc{};
        vot::RenderComponent rc{};
        vot::AnimationComponent ac{};
        vot::RayTracingComponent rtc{};
    };

    struct tUpdateScene{};
}


#endif //VKCELSHADINGRENDERER_EV_STRUCT_H
