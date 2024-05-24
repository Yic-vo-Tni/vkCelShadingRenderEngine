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

    vkInit::vkInit(const std::shared_ptr<vkInitCreateInfo> &createInfo) :

            mInstance([&, appInfo = [&]() {
                return vk::ApplicationInfo{
                        "Yic", VK_MAKE_VERSION(1, 0, 0),
                        "Vot", VK_MAKE_VERSION(1, 0, 0),
                        VK_MAKE_API_VERSION(0, 1, 3, 0)
                };
            }()]() {
                createInfo->addInstanceExtensions(fn::addRequiredExtensions());
                fn::checkInstanceSupport(createInfo->mInstanceExtensions, createInfo->mInstanceLayers);

                return vkCreate("create instance") = [&]() {
                    return vk::createInstanceUnique(
                            vk::InstanceCreateInfo()
                                    .setPApplicationInfo(&appInfo)
                                    .setPEnabledExtensionNames(createInfo->mInstanceExtensions)
                                    .setPEnabledLayerNames(createInfo->mInstanceLayers)
                    );
                };
            }()),
            mDebugMessenger([&](){
                mDynamicDispatcher = vk::DispatchLoaderDynamic(mInstance.get(), vkGetInstanceProcAddr);

                return vkCreate("create debug messenger") = [&](){
                    using tSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
                    using tType = vk::DebugUtilsMessageTypeFlagBitsEXT;
                    return mInstance->createDebugUtilsMessengerEXT(
                            vk::DebugUtilsMessengerCreateInfoEXT()
                                    .setMessageSeverity(tSeverity::eVerbose | tSeverity::eWarning | tSeverity::eError)
                                    .setMessageType(tType::eGeneral | tType::ePerformance | tType::eValidation)
                                    .setPfnUserCallback(debugCallback), nullptr, mDynamicDispatcher);
                };
            }())

            {



    }

}