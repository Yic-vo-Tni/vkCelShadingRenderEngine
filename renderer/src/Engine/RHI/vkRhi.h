//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRHI_H
#define VKCELSHADINGRENDERER_VKRHI_H

#include "Engine/RHI/vkInit.h"
#include "Engine/Core/vkWindow.h"
#include "Engine/RHI/vkSwapchain.h"
#include "Engine/RHI/vkSemaphore.h"
#include "Engine/RHI/FrameRender.h"
#include "Engine/RHI/Descriptor.h"
#include "Engine/RHI/RenderProcessManager.h"



namespace yic {

    class vkRhi {
    public:
        vkRhi();
        ~vkRhi();

        auto FrameLoop() -> bool;
    private:
        sf::Time mStart;
        sf::Time mFrameTime;
        sf::Clock mClock;
        auto beginFrame() -> void;
        auto endFrame() -> void;
    private:

        std::unique_ptr<vkSwapchain> mSwapchain{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRHI_H
