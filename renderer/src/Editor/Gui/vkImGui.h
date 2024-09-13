//
// Created by lenovo on 6/11/2024.
//

#ifndef VKCELSHADINGRENDERER_VKIMGUI_H
#define VKCELSHADINGRENDERER_VKIMGUI_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"

#include "Widget/WidgetManager.h"

namespace yic {

    class vkImGui {
    public:
        vkImGui(vot::string id, vk::Queue graphicsQueue, const uint32_t &queueFamilyIndex);
        ~vkImGui();

        auto render(vk::CommandBuffer& cmd) -> void;

    private:
        auto base() -> void;
        auto callback() -> void;
    private:
        vot::string mId{};
        uint32_t mQueueIndex{};
        vk::Queue mQueue{};
        GLFWwindow* mWindow{};
        vk::Extent2D mExtent{};
        vk::Extent2D* mCurrentExtent{};
        vk::DescriptorPool mDescriptorPool{};

        bool mShowDemo{true};

        ui::WidgetManager mWidgetManager{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKIMGUI_H
