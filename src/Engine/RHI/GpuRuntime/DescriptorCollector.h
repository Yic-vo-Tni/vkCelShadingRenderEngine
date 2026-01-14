//
// Created by lenovo on 11/12/2025.
//

#ifndef VKCELSHADINGRENDERER_DESCRIPTORCOLLECTOR_H
#define VKCELSHADINGRENDERER_DESCRIPTORCOLLECTOR_H

namespace rhi2 {
    class DescriptorCollector {
    public:
        DescriptorCollector();
        ~DescriptorCollector();
    private:
        ev::pVkSetupContext ct;
        vk::DeviceSize offsetAlignment;
    };
} // rhi2

#endif //VKCELSHADINGRENDERER_DESCRIPTORCOLLECTOR_H