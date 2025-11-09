#include <utility>

//
// Created by lenovo on 11/9/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERTARGET_H
#define VKCELSHADINGRENDERER_RENDERTARGET_H

namespace runtime::flow {

    class RenderTarget {
    public:
        RenderTarget() = default;
        explicit RenderTarget(vot::Image_sptr image) : RT(std::move(image)) {};

        RenderTarget& operator=(RenderTarget&& other) noexcept {
            RT = std::move(other.RT);
            return *this;
        }

        ~RenderTarget() = default;

        [[nodiscard]] auto valid() const noexcept -> bool{ return static_cast<bool>(RT); }
        [[nodiscard]] auto va() const noexcept -> vot::Image_sptr { return RT; }
        vot::rhi::Image* operator->() const noexcept { assert(RT && "RenderTarget::image is null"); return RT.get(); }

    public:
        auto drawRender(vot::CommandBuffer& cmd, const vot::ImageDrawCI& drawci, const std::function<void()>& fn) -> void;
        auto drawRendering(vot::CommandBuffer& cmd, const std::function<void()>& fn) -> void;

    private:
        auto beginRendering(vot::CommandBuffer& cmd) -> void;
        auto endRendering(vot::CommandBuffer& cmd) -> void;
    private:
        vot::Image_sptr RT;
    };

    using RT_sptr = std::shared_ptr<RenderTarget>;

} // runtime

#endif //VKCELSHADINGRENDERER_RENDERTARGET_H