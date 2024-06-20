//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_VKFRAMERENDER_H
#define VKCELSHADINGRENDERER_VKFRAMERENDER_H

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkFrameRender {
    public:
        vkFrameRender();
        ~vkFrameRender();

    private:
        auto createRenderPass(const std::vector<vk::AttachmentDescription>& attachDes,
                              const std::vector<vk::SubpassDescription>& subpass,
                              const std::vector<vk::SubpassDependency>& dependency,
                              const std::string& des = {}) -> vk::RenderPass;
        auto createFrameBuffer(vk::RenderPass renderPass, vk::Extent2D extent, const std::vector<std::vector<vk::ImageView>>& attach) -> std::vector<vk::Framebuffer>;

    private:
        vk::Device mDevice{};

    };





























    class vkRenderPassInfo : public std::enable_shared_from_this<vkRenderPassInfo>{
    public:
        auto addAttachDes(const vk::AttachmentDescription& attachDes){
            mAttachDes.emplace_back(attachDes);
            return shared_from_this();
        }

        auto addAttachRef(uint32_t attachIndex, vk::ImageLayout layout){
            auto ref = vk::AttachmentReference().setAttachment(attachIndex)
                                                            .setLayout(layout);
            mAttachRef.emplace_back(ref);
            return shared_from_this();
        }

        auto addSubpass(const vk::SubpassDescription& subpass){
            mSubpass.emplace_back(subpass);
            return shared_from_this();
        }

        auto addDependency(const vk::SubpassDependency& dependency){
            mDependency.emplace_back(dependency);
            return shared_from_this();
        }

        auto create(vk::Device device){
            auto createInfo = vk::RenderPassCreateInfo()
                    .setAttachments(mAttachDes)
                    .setSubpasses(mSubpass)
                    .setDependencies(mDependency);

            mRenderPass = vkCreate("create render pass") = [&] {
                return device.createRenderPass(createInfo);
            };
            return shared_from_this();
        }
    private:
        vk::RenderPass mRenderPass{};

        std::vector<vk::AttachmentDescription> mAttachDes{};
        std::vector<vk::AttachmentReference> mAttachRef{};
        std::vector<vk::SubpassDescription> mSubpass{};
        std::vector<vk::SubpassDependency> mDependency{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKFRAMERENDER_H
