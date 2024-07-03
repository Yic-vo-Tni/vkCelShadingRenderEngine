//
// Created by lenovo on 7/3/2024.
//

#include "vkSemaphore.h"


namespace yic {

    auto vkSemaphore::submit(std::vector<vk::Semaphore>& waitSemaphores) -> void {
        vk::SubmitInfo submitInfo{};
        std::vector<vk::PipelineStageFlags> stages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

        submitInfo.setWaitSemaphores(waitSemaphores)
                .setWaitDstStageMask(stages);

    }

    auto vkSemaphore::createSemaphore(uint32_t& i) -> std::vector<vk::Semaphore> {
        vk::SemaphoreCreateInfo createInfo{};
        std::vector<vk::Semaphore> semaphores;
        semaphores.reserve(i);

        do {
            auto s = vkCreate("create semaphore" + std::to_string(i)) = [&]{
                return mDevice.createSemaphore(createInfo);
            };
            semaphores.push_back(s);
        } while (--i > 0);

        return semaphores;
    }

} // yic