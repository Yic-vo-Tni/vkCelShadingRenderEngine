//
// Created by lenovo on 7/8/2024.
//

#ifndef VKCELSHADINGRENDERER_VKDESCRIPTOR_H
#define VKCELSHADINGRENDERER_VKDESCRIPTOR_H

#include "Engine/Utils/Log.h"

namespace yic {

    class vkDescriptor : public Identifiable{
    public:
        explicit vkDescriptor(const std::string& id) : Identifiable(id) {};
        ~vkDescriptor() override{
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
        [[nodiscard]] inline auto getPipelineLayout() const {
            vk::PipelineLayoutCreateInfo createInfo{
                    {}, mDesSetLayouts
            };
            return vkCreate("create pipeline layout") = [&]{
                return mDevice.createPipelineLayout(createInfo);
            };
        }

        vkDescriptor& addDesSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
        vkDescriptor& createDesPool(std::optional<uint32_t> Reset_MaxSets = std::nullopt);
        vkDescriptor& pushbackDesSets(uint32_t setIndex = 0);
        vkDescriptor& updateDesSet(const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &infos, const size_t& setIndex = 0);


    private:
        vk::Device mDevice;

        uint32_t mMaxSets{};
        uint32_t mIndex{};
        vk::DescriptorPool mDesPool{};
        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> mBindings{};
        std::vector<vk::DescriptorSetLayout> mDesSetLayouts{};
        std::vector<vk::DescriptorPoolSize> mPoolSize{};
        std::vector<vk::DescriptorSet> mDesSet{};
        std::vector<std::vector<vk::WriteDescriptorSet>> mWriteDesSets{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKDESCRIPTOR_H
