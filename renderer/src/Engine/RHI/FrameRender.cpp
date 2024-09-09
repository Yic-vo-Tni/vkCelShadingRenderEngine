//
// Created by lenovo on 6/10/2024.
//

#include "FrameRender.h"

namespace yic {

    vk::RenderPass FrameRender::eColorRenderPass;
    vk::RenderPass FrameRender::eColorDepthStencilRenderPass;

    FrameRender::FrameRender() {
        ct = mg::SystemHub.val<ev::pVkSetupContext>();

        std::vector<vk::AttachmentReference> attachColorRef{
                {0, vk::ImageLayout::eColorAttachmentOptimal},
        };
        vk::AttachmentReference attachColorDepthRef{vk::AttachmentUnused, vk::ImageLayout::eUndefined};

        std::vector<vk::AttachmentDescription> attachDes{
                {{},
                        vk::Format::eR8G8B8A8Unorm, vk::SampleCountFlagBits::e1,
                        vk::AttachmentLoadOp::eClear,
                        vk::AttachmentStoreOp::eStore,
                        vk::AttachmentLoadOp::eDontCare,
                        vk::AttachmentStoreOp::eDontCare,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eShaderReadOnlyOptimal},};
        std::vector<vk::SubpassDescription> subpassColor{
                {{},
                 vk::PipelineBindPoint::eGraphics, {},
                 attachColorRef, {}, &attachColorDepthRef, {}}};

        vk::RenderPassCreateInfo createInfo{{}, attachDes, subpassColor, {}};
        eColorRenderPass = vkCreate("create depth stencil render pass") = [&]{
            return ct.device->createRenderPass(createInfo);};

        attachDes.emplace_back(vk::AttachmentDescription{
                {},
                DepthFormat(), vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eShaderReadOnlyOptimal});

        attachColorDepthRef.setAttachment(1)
                        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        createInfo.setAttachments(attachDes);

        eColorDepthStencilRenderPass = vkCreate("create depth stencil render pass") = [&]{
            return ct.device->createRenderPass(createInfo);};

    }

    FrameRender::~FrameRender() {
        ct.device->destroy(eColorRenderPass);
        ct.device->destroy(eColorDepthStencilRenderPass);
    }

    template<typename T>
    requires(tp::Same_orVector<T, std::shared_ptr<vkImage>>)
    auto FrameRender::createFramebuffers(vk::RenderPass renderPass, const T &imgSptrs) -> std::vector<vk::Framebuffer> {
        std::vector<vk::Framebuffer> framebuffers;

        using type = std::decay_t<T>;

        auto createInfo
                = [&]<tp::Same_orVector<vk::ImageView> viewType>(const std::shared_ptr<vkImage> &sharedPtr,
                                                                 const viewType &views) {
                    return vk::FramebufferCreateInfo().setRenderPass(renderPass)
                            .setWidth(sharedPtr->info_.width)
                            .setHeight(sharedPtr->info_.height)
                            .setLayers(1)
                            .setAttachments(views);
                };

        std::vector<vk::ImageView> views;
        if constexpr (std::is_same_v<type, std::shared_ptr<vkImage>>){
            for (size_t i = 0; i < imgSptrs->info_.imageCount; i++) {
                views.clear();
                views.emplace_back(imgSptrs->imageViews[i]);
                if (imgSptrs->depthImageView){
                    views.emplace_back(imgSptrs->depthImageView);
                }

                framebuffers.emplace_back(get()->ct.device->createFramebuffer(createInfo(imgSptrs, views)));
            }
        } else if constexpr (std::is_same_v<type, std::vector<std::shared_ptr<vkImage>>>){
            for(size_t i = 0; i < imgSptrs.front()->info_.imageCount; i++){
                views.clear();
                for (auto &img: imgSptrs) {
                    auto& view = img->info_.imageCount > i ? img->imageViews[i] : img->imageViews.front();
                    views.emplace_back(view);
                }

                framebuffers.emplace_back(get()->ct.device->createFramebuffer(createInfo(imgSptrs.front(), views)));
            }
        }

        return framebuffers;
    }
    template auto FrameRender::createFramebuffers<std::shared_ptr<vkImage>>(vk::RenderPass renderPass, const std::shared_ptr<vkImage>& imgSptrs) -> std::vector<vk::Framebuffer>;
    template auto FrameRender::createFramebuffers<std::vector<std::shared_ptr<vkImage>>>(vk::RenderPass renderPass, const std::vector<std::shared_ptr<vkImage>>& imgSptrs) -> std::vector<vk::Framebuffer>;


    auto FrameRender::DepthFormat() -> vk::Format {
        auto feature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

        for(const auto& f : { vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint}){
            auto formatProp = ct.physicalDevice->getFormatProperties(f);
            if( (formatProp.optimalTilingFeatures & feature) == feature){
                return f;
            }
        }
        return vk::Format::eD16UnormS8Uint;
    }


} // yic