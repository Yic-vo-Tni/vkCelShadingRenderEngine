//
// Created by lenovo on 7/8/2024.
//

#include "vkDescriptor.h"

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    vkDescriptor &vkDescriptor::addDesSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &bindings) {
        mDevice = EventBus::Get::vkSetupContext().device_ref();

        for(const auto& bind : bindings){
            mPoolSize.emplace_back(bind.descriptorType, bind.descriptorCount);
        }

        mMaxSets++;

        mBindings.push_back(bindings);
        vk::DescriptorSetLayoutCreateInfo createInfo{{}, bindings};

        mDesSetLayouts.emplace_back(
                vkCreate("create descriptor set layout") = [&] {
                    return mDevice.createDescriptorSetLayout(createInfo);
                });

        return *this;
    }

    vkDescriptor &vkDescriptor::createDesPool(std::optional<uint32_t> Reset_MaxSets) {
        if (Reset_MaxSets.has_value()){
            mMaxSets = Reset_MaxSets.value();
        }

        vk::DescriptorPoolCreateInfo createInfo{
            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            mMaxSets, mPoolSize
        };

        mDesPool = vkCreate("create des pool") = [&]{
            return mDevice.createDescriptorPool(createInfo);
        };

        return *this;
    }

    vkDescriptor &vkDescriptor::pushbackDesSets(uint32_t setIndex) {
        vk::DescriptorSetAllocateInfo allocateInfo{mDesPool, mDesSetLayouts};
//        mDesSet.push_back(
//                vkCreate("create des " + std::to_string(setIndex) + "set") = [&] {
//                    return mDevice.allocateDescriptorSets(allocateInfo)[setIndex];
//                }
//        );
        auto des = vkCreate("create des" + std::to_string(setIndex) + "set") = [&]{
            return mDevice.allocateDescriptorSets(allocateInfo);
        };
        mDesSet.insert(mDesSet.end(), des.begin(), des.end());

        return *this;
    }

    vkDescriptor &vkDescriptor::updateDesSet(
            const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &infos, const size_t& setIndex) {
        pushbackDesSets();

        mWriteDesSets.resize(mIndex + 1);
        for(uint32_t i = 0; auto& bind : mBindings[setIndex]){
            std::visit([&](auto&& arg){
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>){
                    mWriteDesSets[mIndex].push_back(vk::WriteDescriptorSet{mDesSet[mIndex], bind.binding, 0, bind.descriptorType, {}, arg});
                } else if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>){
                    mWriteDesSets[mIndex].push_back(vk::WriteDescriptorSet{mDesSet[mIndex], bind.binding, 0, bind.descriptorType, arg});
                }

            }, infos[i]);
            i++;
        }

        mDevice.updateDescriptorSets(mWriteDesSets[mIndex], nullptr);

        mIndex++;

        return *this;
    }

} // yic