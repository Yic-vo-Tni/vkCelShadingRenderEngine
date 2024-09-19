//
// Created by lenovo on 9/17/2024.
//

#ifndef VKCELSHADINGRENDERER_SHADOWMAP_H
#define VKCELSHADINGRENDERER_SHADOWMAP_H

#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/RenderGroup.h"

namespace sc {

    class ShadowMap {
    public:
        ShadowMap();

        auto render(vk::CommandBuffer& cmd, const vot::aabb& aabb, glm::vec3 lightDir) -> void;

    public:
        yic2::Image_sptr mShadowMap;
    private:
        auto build() -> void;
    private:
        flecs::world* ecs;
        glm::vec3 up{0.f, -1.f, 0.f};
        glm::vec3 center{0.f};
        vk::Extent2D mExtent{2560, 1440};
        yic::Descriptor_sptr mDescriptor;
        yic::RenderGroupGraphics_sptr mGraphicsGroup;
    };

    using ShadowMap_uptr = std::unique_ptr<ShadowMap>;

} // sc

#endif //VKCELSHADINGRENDERER_SHADOWMAP_H
