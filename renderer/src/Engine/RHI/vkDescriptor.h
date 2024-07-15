//
// Created by lenovo on 7/8/2024.
//

#ifndef VKCELSHADINGRENDERER_VKDESCRIPTOR_H
#define VKCELSHADINGRENDERER_VKDESCRIPTOR_H

#include "Engine/Utils/Log.h"

namespace yic {

    class vkDescriptor : public Identifiable{
    public:
        struct FixSampler{
            vkGet auto get = []{ return Singleton<FixSampler>::get();};
            FixSampler();
            ~FixSampler();
            static auto createSampler() ->  vk::Sampler;

            static vk::Sampler eDefault;
        };
        struct ImgInfo{
            vk::Sampler sampler{FixSampler::eDefault};
            std::vector<vk::ImageView> imageViews;
            vk::ImageLayout imageLayout{vk::ImageLayout::eShaderReadOnlyOptimal};

            explicit ImgInfo(const std::vector<vk::ImageView>& imageViews) : imageViews(imageViews){}
            ImgInfo(vk::Sampler sampler, const std::vector<vk::ImageView>& imageViews) : sampler(sampler), imageViews(imageViews){}
            ImgInfo(vk::Sampler sampler, const std::vector<vk::ImageView>& imageViews, vk::ImageLayout imageLayout) : sampler(sampler), imageViews(imageViews), imageLayout(imageLayout){}
        };
        struct BufInfo{
            std::vector<vk::Buffer> buffer;
            std::vector<vk::DeviceSize> offset{0};
            std::vector<vk::DeviceSize> range{VK_WHOLE_SIZE};
        };
    public:
        explicit vkDescriptor(const std::string& id) : Identifiable(id) {};
        ~vkDescriptor() override{
            mDevice.destroy(mPipelineLayout);
            if (mDesPool) {
                mDevice.destroyDescriptorPool(mDesPool);
            }

            for (auto& layout : mDesSetLayouts) {
                if (layout) {
                    mDevice.destroyDescriptorSetLayout(layout);
                }
            }
        }

        [[nodiscard]] inline auto& getDescriptorSet() const { return mDesSet;}
        [[nodiscard]] inline auto getPipelineLayout() {
            vk::PipelineLayoutCreateInfo createInfo{
                    {}, mDesSetLayouts
            };

            return mPipelineLayout ? mPipelineLayout : mPipelineLayout = vkCreate("create pipeline layout") = [&] {
                return mDevice.createPipelineLayout(createInfo);
            };
        }

        vkDescriptor& addDesSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
        vkDescriptor& createDesPool(std::optional<uint32_t> Reset_MaxSets = std::nullopt);
        vkDescriptor& pushbackDesSets(uint32_t setIndex = 0);
        vkDescriptor& updateDesSet(const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &infos, const size_t& setIndex = 0);
        vkDescriptor& updateDesSet(uint32_t Reset_MaxSets, const std::vector<std::variant<ImgInfo, BufInfo>> &infos, const size_t& setIndex = 0);

    private:
        vk::Device mDevice;

        uint32_t mMaxSets{};
        uint32_t mIndex{};
        vk::DescriptorPool mDesPool{};
        vk::PipelineLayout mPipelineLayout{};
        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> mBindings{};
        std::vector<vk::DescriptorSetLayout> mDesSetLayouts{};
        std::vector<vk::DescriptorPoolSize> mPoolSize{};
        std::vector<vk::DescriptorSet> mDesSet{};
        std::vector<std::vector<vk::WriteDescriptorSet>> mWriteDesSets{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKDESCRIPTOR_H
