//
// Created by lenovo on 7/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKSEMAPHORE_H
#define VKCELSHADINGRENDERER_VKSEMAPHORE_H

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkSemaphore {
    public:


    private:
        auto createSemaphore(uint32_t& i) -> std::vector<vk::Semaphore>;
        auto submit(std::vector<vk::Semaphore>& waitSemaphores) -> void;
    private:
        vk::Device mDevice;
        vk::Queue mGraphicsQueue;
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKSEMAPHORE_H
