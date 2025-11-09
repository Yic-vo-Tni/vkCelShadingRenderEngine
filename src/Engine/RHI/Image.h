//
// Created by lenovo on 10/4/2024.
//

#ifndef VKCELSHADINGRENDERER_IMAGE_H
#define VKCELSHADINGRENDERER_IMAGE_H

namespace vot::inline rhi {

    struct Image final : Identifiable{
        Image(const smart_vector<vk::Image>& images, const smart_vector<vk::ImageView>& imageViews,
              const smart_vector<VmaAllocation>& allocations, const VmaAllocator& allocator, const ImageCI& c, const string& id);

        Image(const smart_vector<vk::Image>& images, const smart_vector<vk::ImageView>& imageViews,
              const smart_vector<VmaAllocation>& allocations,
              const vk::Image& depthImage, const vk::ImageView& depthImageView,
              const VmaAllocation& depthAlloc, const VmaAllocator& allocator, const ImageCI& c, const string& id);

        ~Image() override;

        [[nodiscard]] auto imageInfo(std::optional<uint32_t> imageViewIndex = std::nullopt,
                                     std::optional<vk::Sampler> sampler = std::nullopt,
                                     vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal) const -> vk::DescriptorImageInfo;
        [[nodiscard]] auto depthImageInfo(std::optional<vk::Sampler> sampler, vk::ImageLayout imageLayout) const -> vk::DescriptorImageInfo;

        smart_vector<vk::Image> images{};
        smart_vector<vk::ImageView> imageViews{};
        smart_vector<VmaAllocation> allocations{};
        smart_vector<vk::Framebuffer> framebuffers{};
        vk::Image depthImage{};
        vk::ImageView depthImageView{};
        VmaAllocation depthAllocation{};
        VmaAllocator allocator{};
        ImageCI config{};
        static inline uint64_t counter{};

        inline static uint32_t *index = nullptr;
        vk::Device device;
        vk::detail::DispatchLoaderDynamic dispatchLoaderDynamic;
    };

} // rhi

#endif //VKCELSHADINGRENDERER_IMAGE_H
