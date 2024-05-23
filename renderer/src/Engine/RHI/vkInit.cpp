//
// Created by lenovo on 5/23/2024.
//

#include "vkInit.h"

namespace yic{

    vkInit::vkInit(const std::shared_ptr<vkInitCreateInfo> &createInfo) :

            mInstance([&, appInfo = []() {
                return vk::ApplicationInfo{
                        "Yic", VK_MAKE_VERSION(1, 0, 0),
                        "Vot", VK_MAKE_VERSION(1, 0, 0),
                        VK_MAKE_API_VERSION(0, 1, 3, 0)
                };
            }()]() {
                createInfo->addInstanceExtensions(fn::addRequiredExtensions());
                fn::checkInstanceSupport(createInfo->mInstanceExtensions, createInfo->mInstanceLayers);

                return vkCreate < vk::UniqueInstance > ("create instance") = [&]() {
                    return vk::createInstanceUnique(
                            vk::InstanceCreateInfo()
                                    .setPApplicationInfo(&appInfo)
                                    .setPEnabledExtensionNames(createInfo->mInstanceExtensions)
                                    .setPEnabledLayerNames(createInfo->mInstanceLayers)
                    );
                };
            }())

//            mDebugMessenger([&]() {
//                return mInstance->createDebugUtilsMessengerEXTUnique({});
//            }())

//            mSurface([&](){
//                return
//            }())

            {
    }

}