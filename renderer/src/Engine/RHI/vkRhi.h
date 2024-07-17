//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRHI_H
#define VKCELSHADINGRENDERER_VKRHI_H

#include "Engine/RHI/vkInit.h"
#include "Engine/Core/vkWindow.h"
#include "Engine/RHI/vkSwapchain.h"
#include "Engine/RHI/vkSemaphore.h"
#include "Engine/RHI/vkFrameRender.h"
#include "Engine/RHI/vkDescriptor.h"
#include "Engine/RHI/RenderProcessManager.h"

#include "SFML/System.hpp"

namespace yic {

    class vkRhi {
    public:
        vkRhi();
        ~vkRhi();

        auto FrameLoop() -> bool;
    private:

    private:
        sf::Clock mClock;
        sf::Time mTimePerFrame;

        std::unique_ptr<vkSwapchain> mSwapchain{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRHI_H
