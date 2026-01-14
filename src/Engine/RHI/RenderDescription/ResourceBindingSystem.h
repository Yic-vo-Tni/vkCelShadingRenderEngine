//
// Created by lenovo on 1/13/2026.
//

#ifndef VKCELSHADINGRENDERER_RESOURCEBINDINGSYSTEM_H
#define VKCELSHADINGRENDERER_RESOURCEBINDINGSYSTEM_H

namespace vot::gfx {
    class ResourceBindingSystem {
    public:
        ResourceBindingSystem();
    private:
        ev::pVkSetupContext ct{};
        vk::PhysicalDeviceDescriptorBufferPropertiesEXT desBufferProperties{};


    };
}

#endif //VKCELSHADINGRENDERER_RESOURCEBINDINGSYSTEM_H
