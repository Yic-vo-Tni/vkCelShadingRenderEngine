//
// Created by lenovo on 5/23/2024.
//

#include "vkInit.h"

namespace yic{

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserdata ){
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    vkInit::vkInit(const std::shared_ptr<vkInitCreateInfo> &createInfo) : mCreateInfo(createInfo),
                                                                          mWindow(std::get<GLFWwindow*>(EventBus::Get::vkWindowContext().window.value())),
                                                                          mInstance(createInstance()),
                                                                          mDebugMessenger(createDebugMessenger()),
                                                                          mPhysicalDevice(pickPhysicalDevice()),
                                                                          mDevice(createLogicalDevice()) {
        mDynamicDispatcher.init(mDevice);
        mQueueFamily->createQueues(mDevice, createInfo->mPriorities.size());

        EventBus::publish(et::vkInitContext{mInstance, mDynamicDispatcher, mDebugMessenger});
        EventBus::publish(et::vkDeviceContext{mPhysicalDevice, mDevice, *mQueueFamily});
    }


    auto vkInit::createInstance() -> vk::Instance {
        auto appInfo = vk::ApplicationInfo{
                "Yic", VK_MAKE_VERSION(1, 0, 0),
                "Vot", VK_MAKE_VERSION(1, 0, 0),
                VK_MAKE_API_VERSION(0, 1, 3, 0)
        };

        mCreateInfo->addInstanceExtensions(fn::addRequiredExtensions());
        fn::checkInstanceSupport(mCreateInfo->mInstanceExtensions, mCreateInfo->mInstanceLayers);

        Rvk_y("create instance") = [&]() {
            return vk::createInstance(
                    vk::InstanceCreateInfo()
                            .setPApplicationInfo(&appInfo)
                            .setPEnabledExtensionNames(mCreateInfo->mInstanceExtensions)
                            .setPEnabledLayerNames(mCreateInfo->mInstanceLayers));
        };
    }

    auto vkInit::createDebugMessenger() -> vk::DebugUtilsMessengerEXT {
        mDynamicDispatcher = vk::DispatchLoaderDynamic(mInstance, vkGetInstanceProcAddr);

        return vkCreate("create debug messenger") = [&](){
            using tSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            using tType = vk::DebugUtilsMessageTypeFlagBitsEXT;
            return mInstance.createDebugUtilsMessengerEXT(
                    vk::DebugUtilsMessengerCreateInfoEXT()
                            .setMessageSeverity(tSeverity::eVerbose | tSeverity::eWarning | tSeverity::eError)
                            .setMessageType(tType::eGeneral | tType::ePerformance | tType::eValidation)
                            .setPfnUserCallback(debugCallback), nullptr, mDynamicDispatcher);
        };
    }

    auto vkInit::pickPhysicalDevice() -> vk::PhysicalDevice {
        auto backup = [&,checkSupport = [&](const vk::PhysicalDevice &phy) -> bool {
                    if (fn::checkPhysicalSupport(phy, mCreateInfo->mPhysicalExtensions)) {
                        if_debug vkInfo("Pick GPU : {0}", phy.getProperties().deviceName);
                        return true;
                    }
                    return false;
                }] -> std::optional<vk::PhysicalDevice> {
            for (std::optional<vk::PhysicalDevice> opt_device; const auto &phy: mInstance.enumeratePhysicalDevices()) {
                if (phy.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                    if (checkSupport(phy)){
                        opt_device = phy;
                        return opt_device;
                    }
                } else if (checkSupport(phy)) {
                    opt_device = phy;
                }
                return opt_device;
            }
            return std::nullopt;
        }();

        if (backup.has_value())
            return backup.value();

        throw std::runtime_error("failed to find support gpu");
    }

    auto vkInit::createLogicalDevice() -> vk::Device {
        mQueueFamily = std::make_shared<QueueFamily>(mPhysicalDevice, QueueType::eGraphics, QueueType::eTransfer);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for(auto& [type , family] : mQueueFamily->getQueueFamilies()){
            if (family.familyIndex.has_value())
                queueCreateInfos.push_back({{}, family.familyIndex.value(), mCreateInfo->mPriorities});
        }

        Rvk_y("create device") = [&](){
            return mPhysicalDevice.createDevice(vk::DeviceCreateInfo()
                                                        .setQueueCreateInfos(queueCreateInfos)
                                                        .setPEnabledExtensionNames(mCreateInfo->mPhysicalExtensions)
                                                        .setPNext(&mCreateInfo->features2));
        };
    }

}