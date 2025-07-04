//
// Created by lenovo on 9/24/2024.
//

#include "VkInit.h"
#include "QueueFamily.h"

#include <utility>
#include "Core/DispatchSystem/SystemHub.h"

namespace rhi {
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserdata ){
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }


    VkInit::VkInit(VkInit::CreateInfo createInfo) : mCreateInfo(std::move(createInfo)),
                                                    mInstance(createInstance()),
                                                    mDebugMessenger(createDebugMessenger()),
                                                    mPhysicalDevice(pickPhysicalDevice()),
                                                    mDevice(createLogicalDevice()){
        mDynamicDispatcher.init(mDevice);
        yic::qFamily->create(mDevice, mCreateInfo.priorities.size());

        yic::systemHub.sto(ev::pVkSetupContext{
            .instance = &mInstance,
            .dynamicDispatcher = &mDynamicDispatcher,
            .debugMessenger = &mDebugMessenger,
            .physicalDevice = &mPhysicalDevice,
            .device = &mDevice,
        });
    }
    VkInit::~VkInit() {
        mDevice.destroy();
        mInstance.destroy(mDebugMessenger, nullptr, mDynamicDispatcher);
        mInstance.destroy();
    }

    auto VkInit::createInstance() -> vk::Instance {
        auto appInfo = vk::ApplicationInfo{
                "Yic", VK_MAKE_VERSION(1, 0, 0),
                "Vot", VK_MAKE_VERSION(1, 0, 0),
                VK_MAKE_API_VERSION(0, 1, 3, 0)
        };

        auto x = addRequiredExtensions();
        mCreateInfo.addInstanceExtensions(addRequiredExtensions());
        checkInstanceSupport(mCreateInfo.instanceExtensions, mCreateInfo.instanceLayers);

        return vot::create("create instance") = [&]() {
            return vk::createInstance(
                    vk::InstanceCreateInfo()
                            .setPApplicationInfo(&appInfo)
                            .setPEnabledExtensionNames(mCreateInfo.instanceExtensions)
                            .setPEnabledLayerNames(mCreateInfo.instanceLayers));
        };
    }

    auto VkInit::createDebugMessenger() -> vk::DebugUtilsMessengerEXT {
        mDynamicDispatcher = vk::DispatchLoaderDynamic(mInstance, vkGetInstanceProcAddr);

        return vot::create("create debug messenger") = [&]{
            using s = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            using t = vk::DebugUtilsMessageTypeFlagBitsEXT;
            return mInstance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(s::eVerbose | s::eWarning | s::eError)
            .setMessageType(t::eGeneral | t::ePerformance | t::eValidation)
            .setPfnUserCallback(debugCallback), nullptr, mDynamicDispatcher);
        };
    }

    auto VkInit::pickPhysicalDevice() -> vk::PhysicalDevice {
        auto checkSupport = [&](const vk::PhysicalDevice& phy) -> bool{
            if (checkPhysicalSupport(phy, mCreateInfo.physicalExtensions)){
                if_debug yic::logger->info("Pick GPU: {0}", static_cast<const char*>(phy.getProperties().deviceName));
                return true;
            }
            return false;
        };
        auto backup = [&]() -> std::optional<vk::PhysicalDevice> {
            std::optional<vk::PhysicalDevice> opt_device;
            for (const auto &phy: mInstance.enumeratePhysicalDevices()) {
                if (phy.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                    if (checkSupport(phy)) {
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

    auto VkInit::createLogicalDevice() -> vk::Device {
        yic::qFamily = QueueFamily::make();
        yic::qFamily->init(mPhysicalDevice);

        vot::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
        for(auto& f : yic::qFamily->getFamilies()){
            queueCreateInfo.emplace_back(vk::DeviceQueueCreateInfo()
                                                 .setQueueFamilyIndex(f.familyIndex.value())
                                                 .setQueuePriorities(mCreateInfo.priorities));
        }

        return vot::create("create device") = [&] {
            return mPhysicalDevice.createDevice(vk::DeviceCreateInfo()
                                                        .setQueueCreateInfos(queueCreateInfo)
                                                        .setPEnabledExtensionNames(mCreateInfo.physicalExtensions)
                                                        .setPNext(&mCreateInfo.features2));
        };
    }

    auto VkInit::addRequiredExtensions() -> vot::vector<const char *> {
        uint32_t count{};
        auto glfwExtensions{glfwGetRequiredInstanceExtensions(&count)};

        return {glfwExtensions, glfwExtensions + count};
    }

    auto
    VkInit::checkInstanceSupport(const vot::vector<const char *> &extensions, const vot::vector<const char *> &layers,
                                 bool print) -> bool {
        auto extensionsSupport = checkSupport(vk::enumerateInstanceExtensionProperties(), extensions,
                                              [](const auto& ex){ return ex.extensionName; }, print);
        auto layersSupport = checkSupport(vk::enumerateInstanceLayerProperties(), layers,
                                          [](const auto& lay){ return lay.layerName; });

        return extensionsSupport && layersSupport;
    }

    auto
    VkInit::checkPhysicalSupport(const vk::PhysicalDevice &physicalDevice, const vot::vector<const char *> &extensions,
                                 bool print) -> bool {
        auto extensionSupport = checkSupport(physicalDevice.enumerateDeviceExtensionProperties(), extensions,
                                             [](const auto& ex){ return ex.extensionName;}, print);

        return extensionSupport;
    }


} // rhi