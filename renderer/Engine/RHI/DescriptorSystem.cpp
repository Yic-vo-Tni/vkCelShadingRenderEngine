//
// Created by lenovo on 6/5/2025.
//

#include "DescriptorSystem.h"

#include <utility>
#include "Core/DispatchSystem/SystemHub.h"

namespace rhi {
    DescriptorSystem::DescriptorSystem() : ct(yic::systemHub.val<ev::pVkSetupContext>()) {
        buildDescriptorPool();
    }

    auto DescriptorSystem::buildDescriptorPool(const uint32_t& maxSets) -> void {
        auto ci = vot::DescriptorPoolCreateInfo()
                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(maxSets)
                .setPoolSizes({
                    {vk::DescriptorType::eUniformBuffer, 0.3f},
                    {vk::DescriptorType::eStorageBuffer, 0.2f},
                    {vk::DescriptorType::eCombinedImageSampler, 0.3f},
                    {vk::DescriptorType::eInputAttachment, 0.1f},
                    {vk::DescriptorType::eAccelerationStructureKHR, 0.1f}
                });

        mDescriptorSets.reserve(maxSets);
        mDescriptorPool = vot::create("create des pool") = [&]{
            return ct.device->createDescriptorPool(ci);
        };
    }

    auto DescriptorSystem::allocUpdateDescriptorSets(
            const vot::vector<vot::vector<vot::vector<DescriptorSystem::descriptorInfo>>> &infos,
            vot::PipelineDescriptorSetLayoutCI2 ci2, const uint32_t &startIndex, const uint32_t &sliceCount) -> vot::DescriptorHandle {
        vot::DescriptorHandle handle;

        vot::vector<vot::vector<vk::WriteDescriptorSet>> mWriteDescriptorSets{};
        for (auto setIndex = 0; setIndex < infos.size(); setIndex++) {
            if (auto update = setIndex + 1; mWriteDescriptorSets.size() < update)
                mWriteDescriptorSets.resize(update);

            pushbackDescriptorHandle(ci2, startIndex, handle, sliceCount);

            for (auto desIndex = 0; desIndex < infos[setIndex].size(); desIndex++) {
                const auto& bindings = ci2.setLayoutBindings.at(desIndex + startIndex);

                for (auto bindingIndex = 0; bindingIndex < bindings.size(); bindingIndex++) {
                    const auto &binding = bindings[bindingIndex];
                    const auto &descriptorInfo = infos[setIndex][desIndex][bindingIndex];
                    const auto &writeSetIndex = setIndex * (ci2.desSetLayouts.size() - startIndex) + desIndex;

                    std::visit([&](auto &&arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ handle.pSet[writeSetIndex], binding.binding, 0, binding.descriptorType, {}, arg});
                        if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ handle.pSet[writeSetIndex], binding.binding, 0, binding.descriptorType, arg});
                        if constexpr (std::is_same_v<T, vk::WriteDescriptorSetAccelerationStructureKHR>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ handle.pSet[writeSetIndex], binding.binding, 0, binding.descriptorCount, binding.descriptorType, {}, {}, {}, &arg});
                    }, descriptorInfo);
                }
            }
            ct.device->updateDescriptorSets(mWriteDescriptorSets[setIndex], nullptr);
        }

        return handle;
    }

    auto DescriptorSystem::updateDescriptorSets(const vot::vector<vot::vector<vot::vector<DescriptorSystem::descriptorInfo>>> &infos, vot::PipelineDescriptorSetLayoutCI2 ci2,
                                                vot::DescriptorHandle &handle) -> void {
        vot::vector<vot::vector<vk::WriteDescriptorSet>> mWriteDescriptorSets{};
        for (auto setIndex = 0; setIndex < infos.size(); setIndex++) {
            if (auto update = setIndex + 1; mWriteDescriptorSets.size() < update)
                mWriteDescriptorSets.resize(update);

            for (auto desIndex = 0; desIndex < infos[setIndex].size(); desIndex++) {
                const auto& bindings = ci2.setLayoutBindings.at(desIndex + handle.startIndex);

                for (auto bindingIndex = 0; bindingIndex < bindings.size(); bindingIndex++) {
                    const auto &binding = bindings[bindingIndex];
                    const auto &descriptorInfo = infos[setIndex][desIndex][bindingIndex];
                    const auto &writeSetIndex = setIndex * (ci2.desSetLayouts.size() - handle.startIndex) + desIndex;

                    std::visit([&](auto &&arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ handle.pSet[writeSetIndex], binding.binding, 0, binding.descriptorType, {}, arg});
                        if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ handle.pSet[writeSetIndex], binding.binding, 0, binding.descriptorType, arg});
                        if constexpr (std::is_same_v<T, vk::WriteDescriptorSetAccelerationStructureKHR>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ handle.pSet[writeSetIndex], binding.binding, 0, binding.descriptorCount, binding.descriptorType, {}, {}, {}, &arg});
                    }, descriptorInfo);
                }
            }
            ct.device->updateDescriptorSets(mWriteDescriptorSets[setIndex], nullptr);
        }
    }

    auto DescriptorSystem::pushbackDescriptorHandle(const vot::PipelineDescriptorSetLayoutCI2& ci2, const uint32_t &startIndex,
                                                    vot::DescriptorHandle &handle, const uint32_t &sliceCount) -> void {
        auto& desSetLayouts = ci2.desSetLayouts;

        auto allocInfo = vk::DescriptorSetAllocateInfo()
                .setDescriptorPool(mDescriptorPool)
                .setDescriptorSetCount(sliceCount == 0 ? desSetLayouts.size() - startIndex : sliceCount)
                .setPSetLayouts(desSetLayouts.data() + startIndex);

        auto sets = ct.device->allocateDescriptorSets(allocInfo);

        auto baseIndex = static_cast<uint32_t>(mDescriptorSets.size());
        mDescriptorSets.insert(mDescriptorSets.end(), sets.begin(), sets.end());

        if (handle.pSet == nullptr){
            handle.setCount = static_cast<uint32_t>(sets.size());
            handle.startIndex = startIndex;
            handle.pSet = mDescriptorSets.data() + baseIndex;
        } else {
            ++handle.setCount;
        }
    }

    auto DescriptorSystem::allocUpdateDescriptorSets(const vot::vector<vot::vector<descriptorInfo>> &infos,
                                                vot::PipelineDescriptorSetLayoutCI2 ci2,
                                                const uint32_t &startIndex, const uint32_t &sliceCount) -> vot::DescriptorHandle {
        vot::vector<vot::vector<vot::vector<descriptorInfo>>> fo{infos};
        return allocUpdateDescriptorSets(fo, std::move(ci2), startIndex, sliceCount);
    }

    auto DescriptorSystem::allocUpdateDescriptorSets(const vot::vector<descriptorInfo> &infos,
                                                vot::PipelineDescriptorSetLayoutCI2 ci2,
                                                const uint32_t &startIndex, const uint32_t &sliceCount) -> vot::DescriptorHandle {
        vot::vector<vot::vector<descriptorInfo>> fo{infos};
        return allocUpdateDescriptorSets(fo, std::move(ci2), startIndex, sliceCount);
    }


    auto DescriptorSystem::updateDescriptorSets(const vot::vector<vot::vector<descriptorInfo>> &infos,
                                                vot::PipelineDescriptorSetLayoutCI2 ci2,
                                                vot::DescriptorHandle &handle) -> void {
        vot::vector<vot::vector<vot::vector<descriptorInfo>>> fo{infos};
        updateDescriptorSets(fo, std::move(ci2), handle);
    }

    auto DescriptorSystem::updateDescriptorSets(const vot::vector<descriptorInfo> &infos,
                                                vot::PipelineDescriptorSetLayoutCI2 ci2,
                                                vot::DescriptorHandle &handle) -> void {
        vot::vector<vot::vector<descriptorInfo>> fo{infos};
        updateDescriptorSets(fo, std::move(ci2), handle);
    }

} // rhi