//
// Created by lenovo on 6/11/2024.
//

#ifndef VKCELSHADINGRENDERER_VKIMGUI_H
#define VKCELSHADINGRENDERER_VKIMGUI_H

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkImGui {
    public:
        vkImGui();
        ~vkImGui();

        auto beginRenderImGui() -> void;
        auto endRenderImGui(const vk::CommandBuffer& cmd) -> void;

    private:
        vk::DescriptorPool mDescriptorPool{};

        bool mShowDemo{true};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKIMGUI_H
