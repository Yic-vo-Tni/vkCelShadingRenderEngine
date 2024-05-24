//
// Created by lenovo on 4/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKCOMMON_H
#define VKCELSHADINGRENDERER_VKCOMMON_H

#include "Engine/Utils/Log.h"

enum class QueueType {
    eGraphics, eTransfer,
};


namespace fn{

    inline auto addRequiredExtensions(){
        uint32_t count{};
        auto glfwExtensions{glfwGetRequiredInstanceExtensions(&count)};

        return std::vector<const char*>(glfwExtensions, glfwExtensions + count);
    }

    template<typename vkProperties, typename NameFunc>
    inline bool checkSupport(const vkProperties& properties, std::vector<const char*> requiredExs, NameFunc name, bool print = false){
        std::vector<const char*> foundExs;
        for(auto& pro : properties){
            auto it = std::find_if(requiredExs.begin(), requiredExs.end(), [&](const char* requiredEx){
                return strcmp(name(pro), requiredEx) == 0;
            });
            if (it != requiredExs.end()){
                if_debug{ vkWarn("Enable extension or layer : {0}", name(pro));}
                foundExs.emplace_back(*it);
            } else{
                if_debug{ if (print) { vkTrance(name(pro));}}
            }
        }

        for(auto& found : foundExs){
            requiredExs.erase(std::remove(requiredExs.begin(), requiredExs.end(), found), requiredExs.end());
        }

        if (!requiredExs.empty()){
            if_debug{ vkInfo("the following required items were not found: "); }
            for (const auto& missingEx : requiredExs) {
                if_debug{ vkError(missingEx); }
            }
            return false;
        }

        return true;
    }

    inline bool checkInstanceSupport(const std::vector<const char*>& extensions, const std::vector<const char*>& layers, bool print = false){
        auto extensionsSupport = checkSupport(vk::enumerateInstanceExtensionProperties(), extensions,
                     [](const auto& ex){ return ex.extensionName; }, print);
        auto layersSupport = checkSupport(vk::enumerateInstanceLayerProperties(), layers,
                     [](const auto& lay){ return lay.layerName; });

        return extensionsSupport && layersSupport;
    }

    inline bool checkPhysicalSupport(const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions, bool print = false){
        auto extensionSupport = checkSupport(physicalDevice.enumerateDeviceExtensionProperties(), extensions,
                                             [](const auto& ex){ return ex.extensionName;}, print);

        return extensionSupport;
    }

    inline std::optional<uint32_t> findQueueFamily(vk::PhysicalDevice physicalDevice, QueueType type, bool print = false){
        auto families = physicalDevice.getQueueFamilyProperties();
        if (print) {
            if_debug { vkTrance("physical device support {0} queue families! ", families.size()); }
            for (const auto &queueFamily: families) {
                std::cout << "\tQueue Flags: ";

                if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                    std::cout << "Graphics ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                    std::cout << "Compute ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
                    std::cout << "Transfer ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eSparseBinding) {
                    std::cout << "Sparse Binding ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eProtected) {
                    std::cout << "Protected ";
                }

                std::cout << "\n";
            }
        }

        for(size_t i = 0; const auto& f : families){
            if (type == QueueType::eGraphics){
                if (f.queueFlags & vk::QueueFlagBits::eGraphics)
                    return i;
            }

            if (type == QueueType::eTransfer){
                if ((f.queueFlags & vk::QueueFlagBits::eTransfer) &&
                    !(f.queueFlags & vk::QueueFlagBits::eGraphics) &&
                    !(f.queueFlags & vk::QueueFlagBits::eCompute)){
                    return i;
                }
            }

            i++;
        }

        return std::nullopt;
    }


};

#endif //VKCELSHADINGRENDERER_VKCOMMON_H











