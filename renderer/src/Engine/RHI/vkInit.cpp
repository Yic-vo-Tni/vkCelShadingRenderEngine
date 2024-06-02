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

    vkInit::vkInit(const std::shared_ptr<vkInitCreateInfo> &createInfo, GLFWwindow* w) : mWindow(w)

            , mInstance([&, appInfo = [&]() {
                return vk::ApplicationInfo{
                        "Yic", VK_MAKE_VERSION(1, 0, 0),
                        "Vot", VK_MAKE_VERSION(1, 0, 0),
                        VK_MAKE_API_VERSION(0, 1, 3, 0)
                };
            }()]() {
                createInfo->addInstanceExtensions(fn::addRequiredExtensions());
                fn::checkInstanceSupport(createInfo->mInstanceExtensions, createInfo->mInstanceLayers);

                return vkCreate("create instance") = [&]() {
                    return vk::createInstance(
                            vk::InstanceCreateInfo()
                                    .setPApplicationInfo(&appInfo)
                                    .setPEnabledExtensionNames(createInfo->mInstanceExtensions)
                                    .setPEnabledLayerNames(createInfo->mInstanceLayers)
                    );
                };
            }())

            , mDebugMessenger([&](){
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
            }())

            , mSurface([&](){
                return vkCreate("create surface") = [&](){
                    auto w = EventBus::getState<EventTypes::WindowContext>().window.get();
                    if (VkSurfaceKHR tempSurface;
                            glfwCreateWindowSurface(mInstance, w, nullptr, &tempSurface) == VK_SUCCESS){
                        return tempSurface;
                    } else{
                        exit(1);
                    }
                };
            }())

            , mPhysicalDevice([&,
                               backup = [&,
                                         checkSupport = [&
                                                         ](const vk::PhysicalDevice& phy) -> bool {
                if (fn::checkPhysicalSupport(phy, createInfo->mPhysicalExtensions)) {
                    if_debug vkInfo("Pick GPU : {0}", phy.getProperties().deviceName);
                    return true;
                }
                return false;
            }] -> std::optional<vk::PhysicalDevice>{
                for(std::optional<vk::PhysicalDevice> opt_device;const auto& phy : mInstance.enumeratePhysicalDevices()){
                    if (phy.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu){
                        if (checkSupport(phy))
                            opt_device = phy;
                        return opt_device;
                    } else if(!opt_device.has_value() && checkSupport(phy)){
                        opt_device = phy;
                        return opt_device;
                    }
                }
                return std::nullopt;
            }] -> vk::PhysicalDevice{
                if (auto re = backup(); re.has_value()){
                    return re.value();
                } else { vkError("failed to find support gpu"); exit(0); }
            }())

            , mDevice([&]{
                mQueueFamilies.emplace(QueueType::eGraphics, QueueFamily{{}, fn::findQueueFamily(mPhysicalDevice, QueueType::eGraphics)});
                mQueueFamilies.emplace(QueueType::eTransfer, QueueFamily{{}, fn::findQueueFamily(mPhysicalDevice, QueueType::eTransfer)});

                std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

                for(auto& [type , family] : mQueueFamilies){
                    if (family.familyIndex.has_value())
                        queueCreateInfos.push_back({{}, family.familyIndex.value(), createInfo->mPriorities});
                }

                return vkCreate("create device") = [&](){
                    return mPhysicalDevice.createDevice(vk::DeviceCreateInfo()
                                                                   .setQueueCreateInfos(queueCreateInfos)
                                                                   .setPEnabledExtensionNames(createInfo->mPhysicalExtensions)
                                                                   .setPNext(&createInfo->features2));
                };
            }())

            {
                mDynamicDispatcher.init(mDevice);
                for(auto& [type, family] : mQueueFamilies){
                    family.createQueues(mDevice, createInfo->mPriorities.size());
                }
    }

}