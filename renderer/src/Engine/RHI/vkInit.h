//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_VKINIT_H
#define VKCELSHADINGRENDERER_VKINIT_H

#include "vkStruct.h"

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkInitCreateInfo;

    class vkInit {
    public:
        explicit vkInit(const std::shared_ptr<vkInitCreateInfo>& createInfo);
        ~vkInit() {
            mDevice.destroy();
            mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr, mDynamicDispatcher);
            mInstance.destroy();
        }

        vkGet auto get = [](const std::shared_ptr<vkInitCreateInfo>& createInfo = {}){
            return Singleton<vkInit>::get(createInfo);
        };

        [[nodiscard]] inline static const auto& GetInstance() { return get()->mInstance;}
        [[nodiscard]] inline static const auto& GetDynamicDispatcher() { return get()->mDynamicDispatcher;}
        [[nodiscard]] inline static const auto& GetDevice() { return get()->mDevice;}

    private:
        auto createInstance() -> vk::Instance;
        auto createDebugMessenger() -> vk::DebugUtilsMessengerEXT;
        auto pickPhysicalDevice() -> vk::PhysicalDevice;
        auto createLogicalDevice() -> vk::Device;
    private:
        std::shared_ptr<vkInitCreateInfo> mCreateInfo{};
        vk::Instance mInstance{};
        vk::DispatchLoaderDynamic mDynamicDispatcher{};
        vk::DebugUtilsMessengerEXT mDebugMessenger{};
        vk::PhysicalDevice mPhysicalDevice{};
        std::shared_ptr<QueueFamily> mQueueFamily{};
        vk::Device mDevice{};
    };

    class vkInitCreateInfo : public std::enable_shared_from_this<vkInitCreateInfo>{
    public:
        auto addInstanceExtensions(const auto& extensions){
            addExOrLay(extensions, mInstanceExtensions);
            return shared_from_this();
        }

        auto addInstanceLayers(const auto& layers){
            addExOrLay(layers, mInstanceLayers);
            return shared_from_this();
        }

        template<typename Feature = std::nullopt_t>
        auto addPhysicalExtensions(const auto& extensions, Feature* feature = nullptr){
            addExOrLay(extensions, mPhysicalExtensions);
            if constexpr (!std::is_same_v<Feature, std::nullopt_t>)
                addFeatureToChain(feature);
            return shared_from_this();
        }

        auto setQueuesPriority(const std::vector<float>& priorities){
            mPriorities = priorities;
            return shared_from_this();
        }

    private:
        template<typename T>
        auto& addExOrLay(const T& extensions, auto& target){
            if constexpr (std::is_same_v<T, const char*> || std::is_array_v<T>){
                target.emplace_back(extensions);
            } else if constexpr (std::is_same_v<T, std::vector<const char*>>){
                target.insert(target.end(), extensions.begin(), extensions.end());
            }
            return *this;
        }

        template<typename feature>
        bool addFeatureToChain(feature* f){
            f->pNext = features2.pNext;
            features2.pNext = f;

            return true;
        }

    public:
        vk::PhysicalDeviceFeatures2 features2{};
        std::vector<float> mPriorities{1.f};
        std::vector<const char*> mInstanceExtensions{},
                mInstanceLayers{},
                mPhysicalExtensions{};
    };

}


#endif //VKCELSHADINGRENDERER_VKINIT_H
