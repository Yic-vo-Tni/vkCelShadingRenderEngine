//
// Created by lenovo on 10/4/2024.
//

#ifndef VKCELSHADINGRENDERER_IMAGE_H
#define VKCELSHADINGRENDERER_IMAGE_H

namespace vot::inline rhi {

    struct Image : public Identifiable{
    public:
        Image(const vot::smart_vector<vk::Image>& images, const vot::smart_vector<vk::ImageView>& imageViews,
              const vot::smart_vector<VmaAllocation>& allocations, VmaAllocator& allocator, const vot::ImageCI& c, const vot::string& id);

        Image(const vot::smart_vector<vk::Image>& images, const vot::smart_vector<vk::ImageView>& imageViews,
              const vot::smart_vector<VmaAllocation>& allocations,
              const vk::Image& depthImage, const vk::ImageView& depthImageView,
              const VmaAllocation& depthAlloc, VmaAllocator& allocator, const vot::ImageCI& c, const vot::string& id);

        ~Image() override;

        [[nodiscard]] auto imageInfo(std::optional<uint32_t> imageViewIndex = std::nullopt,
                                     std::optional<vk::Sampler> sampler = std::nullopt,
                                     //std::optional<std::optional<vk::Sampler>> sampler = std::nullopt,
                                     vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal) const -> vk::DescriptorImageInfo;

        auto beginRendering(vot::CommandBuffer& cmd, vk::Rect2D rect2D = vk::Rect2D{{0, 0}, vot::Resolutions::eQHDExtent}) -> void;
        auto endRendering(vot::CommandBuffer& cmd) -> void;
        auto drawRendering(vot::CommandBuffer &cmd, const std::function<void()> &fn) -> void;

        auto drawRender(vot::CommandBuffer& cmd, const vot::ImageDrawCI& ci, const std::function<void()>& fn)  -> void;
    public:
        vot::smart_vector<vk::Image> images{};
        vot::smart_vector<vk::ImageView> imageViews{};
        vot::smart_vector<VmaAllocation> allocations{};
        vot::smart_vector<vk::Framebuffer> framebuffers{};
        vk::Image depthImage{};
        vk::ImageView depthImageView{};
        VmaAllocation depthAllocation{};
        VmaAllocator allocator{};
        vot::ImageCI config{};
        static inline uint64_t counter{};

    protected:
        inline static uint32_t *index = nullptr;
        vk::Device device;
        vk::DispatchLoaderDynamic dispatchLoaderDynamic;
        uint32_t activeIndex{};
    };

} // rhi

#endif //VKCELSHADINGRENDERER_IMAGE_H
