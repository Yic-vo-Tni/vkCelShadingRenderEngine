//
// Created by lenovo on 6/2/2024.
//

#ifndef VKCELSHADINGRENDERER_VKSTRUCTURE_H
#define VKCELSHADINGRENDERER_VKSTRUCTURE_H

#include "vkCommon.h"

namespace yic{

    struct QueueFamily {
        struct Family {
            std::vector<vk::Queue> queues{};
            std::optional<uint32_t> familyIndex{};

            bool createQueues(vk::Device device, uint32_t queueCount, const std::string& des) {
                if (familyIndex.has_value()) {
                    queues.resize(queueCount);
                    for (uint32_t i = 0; i < queueCount; i++) {
                        queues[i] = device.getQueue(familyIndex.value(), i);
                        vkTrance("create " + std::to_string(i) + " " + des + " queue successfully");
                    }
                }
                return true;
            }

        };

        template<typename ...Args>
        explicit QueueFamily(vk::PhysicalDevice phy, Args ...args){
            (addQueueFamily(phy, args), ...);
        }

        void createQueues(vk::Device device, uint32_t queueCount){
            for(auto& [type, family] : mQueueFamilies){
                if (type == QueueType::eGraphics)
                    family.createQueues(device, queueCount, "graphics");
                if (type == QueueType::eTransfer)
                    family.createQueues(device, queueCount, "transfer");
            }
        }

        [[nodiscard]] inline auto& getQueueFamilies() { return mQueueFamilies;}
        [[nodiscard]] inline auto getImGuiGraphicsFamilyIndex() { return mQueueFamilies[QueueType::eGraphics].familyIndex.value();}
        [[nodiscard]] inline const auto getImGuiGraphicsFamilyIndex() const { return mQueueFamilies.at(QueueType::eGraphics).familyIndex.value();}
        [[nodiscard]] inline auto getMainGraphicsFamilyIndex() { return mQueueFamilies[QueueType::eGraphics].familyIndex.value();}
        [[nodiscard]] inline const auto getMainGraphicsFamilyIndex() const { return mQueueFamilies.at(QueueType::eGraphics).familyIndex.value();}
        [[nodiscard]] inline auto& getImGuiGraphicsQueue() { return mQueueFamilies.at(QueueType::eGraphics).queues.at(0);}
        [[nodiscard]] inline const auto& getImGuiGraphicsQueue() const { return mQueueFamilies.at(QueueType::eGraphics).queues.at(0);}
        [[nodiscard]] inline auto& getMainGraphicsQueue() { return mQueueFamilies.at(QueueType::eGraphics).queues.at(1);}
        [[nodiscard]] inline const auto& getMainGraphicsQueue() const { return mQueueFamilies.at(QueueType::eGraphics).queues.at(1);}
    private:
        void addQueueFamily(vk::PhysicalDevice phy, QueueType type) {
            mQueueFamilies.emplace(type, Family{.familyIndex = fn::findQueueFamily(phy, type)});
        }

        std::unordered_map<QueueType, Family> mQueueFamilies{};
    };
}

#endif //VKCELSHADINGRENDERER_VKSTRUCTURE_H
