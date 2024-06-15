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

#include "Editor/UI/Gui/vkImGui.h"

namespace yic {

    class vkRhi {
    public:
        vkRhi();
        ~vkRhi();

        auto run() -> bool ;

    private:
        std::unique_ptr<vkSwapchain> mSwapchain{};
        std::unique_ptr<vkFrameRender> mFrameRender{};
        std::unique_ptr<vkCommand> mCommand{};
        std::unique_ptr<vkImGui> mImGui{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRHI_H
