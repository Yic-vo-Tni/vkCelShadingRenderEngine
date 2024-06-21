//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRHI_H
#define VKCELSHADINGRENDERER_VKRHI_H

#include "vkInit.h"
#include "Engine/Core/vkWindow.h"
#include "Engine/RHI/vkSwapchain.h"
#include "Engine/RHI/vkFrameRender.h"
#include "Engine/RHI/vkCommand.h"
#include "Engine/RHI/vkPipeline.h"

#include "Editor/UI/Gui/vkImGui.h"

namespace yic {

    class vkRhi {
    public:
        vkRhi();
        ~vkRhi();

        auto ImGuiFrameLoop() -> bool;

    private:
        std::unique_ptr<vkSwapchain> mImGuiSwapchain{};
        std::unique_ptr<vkCommand> mImGuiCommand{};
        std::unique_ptr<vkImGui> mImGui{};

        vk::PipelineLayout layout;
        std::unique_ptr<vkPipeline<Graphics>> pipeline;
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRHI_H
