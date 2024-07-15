//
// Created by lenovo on 6/11/2024.
//

#ifndef VKCELSHADINGRENDERER_VKIMGUI_H
#define VKCELSHADINGRENDERER_VKIMGUI_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"

#include "Widget/Widget.h"

namespace yic {

    class vkImGui {
    public:
        vkImGui(std::string id, vk::Queue graphicsQueue, const uint32_t &queueFamilyIndex);
        ~vkImGui();

        auto render(vk::CommandBuffer& cmd) -> void;

    private:
        auto base() -> void;
        auto callback() -> void;
    private:
        std::string mId{};
        uint32_t mQueueIndex{};
        vk::Queue mQueue{};
        GLFWwindow* mWindow{};
        vk::DescriptorPool mDescriptorPool{};

        bool mShowDemo{true};

        ui::Widget mWidget{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKIMGUI_H
