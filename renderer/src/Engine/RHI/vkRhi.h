//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRHI_H
#define VKCELSHADINGRENDERER_VKRHI_H

#include "vkInit.h"
#include "Engine/Core/vkWindow.h"
#include "Engine/RHI/vkSwapchain.h"
#include "Engine/RHI/vkSemaphore.h"
#include "Engine/RHI/vkFrameRender.h"
#include "Engine/RHI/vkDescriptor.h"
#include "Engine/RHI/vkCommand.h"
#include "Engine/RHI/vkPipeline.h"
#include "Engine/RHI/vkAllocator.h"

#include "Editor/Gui/vkImGui.h"

#include "SFML/System.hpp"

namespace yic {

    class vkRhi {
    public:
        vkRhi();
        ~vkRhi();

        auto FrameLoop() -> bool;


    private:
        auto t() -> void;

    private:
        sf::Clock mClock;
        sf::Time mTimePerFrame;

        std::unique_ptr<vkSwapchain> mSwapchain{};
        std::unique_ptr<vkImGui> mImGui{};

        std::unique_ptr<vkCommand> t_cmd{};
        ///
        vkImg_sptr off_image;
        vk::Extent2D ext;
        vk::RenderPass renderPass;
        std::vector<vk::Framebuffer> frameBuffers;
        std::shared_ptr<vkDescriptor> descriptor;
        std::unique_ptr<vkPipeline<Graphics>> pipeline;
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRHI_H
