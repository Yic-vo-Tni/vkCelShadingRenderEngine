//
// Created by lenovo on 1/13/2026.
//

#include "ResourceBindingSystem.h"

#include "Core/DispatchSystem/SystemHub.h"

namespace vot::gfx {
    ResourceBindingSystem::ResourceBindingSystem() {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        vk::PhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.pNext = &desBufferProperties;
        ct.physicalDevice->getProperties2(&deviceProperties);
        if (desBufferProperties.maxDescriptorBufferBindings <= 8) {
            yic::logger->warn("Descriptor buffer bindings : {0}", desBufferProperties.maxDescriptorBufferBindings);
        }
    }
}
