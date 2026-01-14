//
// Created by lenovo on 11/12/2025.
//

#include "DescriptorCollector.h"

#include "Core/DispatchSystem/SystemHub.h"

namespace rhi2 {

    DescriptorCollector::DescriptorCollector() {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        auto properties = vk::PhysicalDeviceDescriptorBufferPropertiesEXT();
        auto props2 = vk::PhysicalDeviceProperties2{{}, &properties};
        ct.physicalDevice->getProperties2(&props2);
        offsetAlignment = properties.descriptorBufferOffsetAlignment;
    }

    DescriptorCollector::~DescriptorCollector() = default;

} // rhi2