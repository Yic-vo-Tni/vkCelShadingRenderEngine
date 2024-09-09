//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_FRAMERENDER_H
#define VKCELSHADINGRENDERER_FRAMERENDER_H

#include "Engine/Utils/Log.h"
#include "Engine/Utils/TypeConcepts.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

#include "Engine/RHI/vkAsset.h"

namespace yic {

    class FrameRender {
    public:
        vkGet auto get = [](){ return Singleton<FrameRender>::get(); };

        FrameRender();
        ~FrameRender();

        template<typename T> requires(tp::Same_orVector<T, std::shared_ptr<vkImage>>)
        static auto createFramebuffers(vk::RenderPass renderPass, const T& imgSptrs) -> std::vector<vk::Framebuffer>;

        template<typename...Args> requires tp::Same_Args<std::shared_ptr<vkImage>, Args...>
        static auto createFramebuffers(vk::RenderPass renderPass, const Args&... imgSptr) -> std::vector<vk::Framebuffer>{
            std::vector<std::shared_ptr<vkImage>> imgSptrs;

            (..., (imgSptrs.emplace_back(imgSptr)));

            return createFramebuffers(renderPass, imgSptrs);;
        };

    private:
        auto DepthFormat() -> vk::Format;
    public:
        static vk::RenderPass eColorRenderPass;
        static vk::RenderPass eColorDepthStencilRenderPass;

    private:
        ev::pVkSetupContext ct;
    };





























} // yic

#endif //VKCELSHADINGRENDERER_FRAMERENDER_H
