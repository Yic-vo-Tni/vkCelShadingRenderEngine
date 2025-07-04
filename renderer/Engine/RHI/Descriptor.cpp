//
// Created by lenovo on 10/3/2024.
//

#include "Descriptor.h"
#include "Core/DispatchSystem/SystemHub.h"

#include <utility>

namespace rhi {
    Descriptor::Descriptor() : ct(yic::systemHub.val<ev::pVkSetupContext>()) {

    }

    Descriptor::Descriptor(vot::PipelineDescriptorSetLayoutCI pipelineDescriptorSetLayoutCi)
            : ct(yic::systemHub.val<ev::pVkSetupContext>()),
              mPipelineDescriptorSetLayoutCI(std::move(pipelineDescriptorSetLayoutCi)) {

    }

    Descriptor::~Descriptor() {
        ct.device->destroy(mDescriptorPool);
    }

    auto Descriptor::createDescriptorPool(const std::optional<uint32_t> &resetMaxSets) -> void {
        if (resetMaxSets.has_value())
            mPipelineDescriptorSetLayoutCI.maxSets = resetMaxSets.value();

        auto ci = vk::DescriptorPoolCreateInfo()
                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setPoolSizes(mPipelineDescriptorSetLayoutCI.poolSizes)
                .setMaxSets(mPipelineDescriptorSetLayoutCI.maxSets);

        mDescriptorPool = vot::create("create des pool") = [&]{
            return ct.device->createDescriptorPool(ci);
        };
    }

    auto Descriptor::pushbackDescriptorSets() -> void {
        auto allocInfo = vk::DescriptorSetAllocateInfo()
                .setDescriptorPool(mDescriptorPool)
                .setSetLayouts(mPipelineDescriptorSetLayoutCI.desSetLayouts);

        auto sets = ct.device->allocateDescriptorSets(allocInfo);
        for(auto& set : sets){ mDescriptorSets.emplace_back(set); }
    }

    auto Descriptor::updateDescriptorSets(const vot::vector<vot::vector<vot::vector<descriptorInfo>>> &infos) -> std::shared_ptr<Descriptor> {
        createDescriptorPool((uint32_t )infos.size());

        for (auto setIndex = 0; setIndex < infos.size(); setIndex++) {
            if (auto update = setIndex + 1; mWriteDescriptorSets.size() < update)
                mWriteDescriptorSets.resize(update);
            pushbackDescriptorSets();
            for (auto desIndex = 0; desIndex < infos[setIndex].size(); desIndex++) {
                const auto &bindings = mPipelineDescriptorSetLayoutCI.desSetBindings[desIndex];

                for (auto bindingIndex = 0; bindingIndex < bindings.size(); bindingIndex++) {
                    const auto &binding = bindings[bindingIndex];
                    const auto &descriptorInfo = infos[setIndex][desIndex][bindingIndex];
                    const auto &writeSetIndex = setIndex * mPipelineDescriptorSetLayoutCI.desSetLayouts.size() + desIndex;

                    std::visit([&](auto &&arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ mDescriptorSets[writeSetIndex], binding.binding, 0, binding.descriptorType, {}, arg});
                        if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ mDescriptorSets[writeSetIndex], binding.binding, 0, binding.descriptorType, arg});
                        if constexpr (std::is_same_v<T, vk::WriteDescriptorSetAccelerationStructureKHR>)
                            mWriteDescriptorSets[setIndex].emplace_back(vk::WriteDescriptorSet{ mDescriptorSets[writeSetIndex], binding.binding, 0, binding.descriptorCount, binding.descriptorType, {}, {}, {}, &arg});
                    }, descriptorInfo);
                }
            }
            ct.device->updateDescriptorSets(mWriteDescriptorSets[setIndex], nullptr);
        }

        return shared_from_this();
    }

    auto Descriptor::updateDescriptorSets(const vot::vector<vot::vector<Descriptor::descriptorInfo>> &infos) -> std::shared_ptr<Descriptor> {
        vot::vector<vot::vector<vot::vector<descriptorInfo>>> fo{infos};
        updateDescriptorSets(fo);

        return shared_from_this();
    }

    auto Descriptor::updateDescriptorSets(const vot::vector<Descriptor::descriptorInfo> &infos) -> std::shared_ptr<Descriptor> {
        vot::vector<vot::vector<descriptorInfo>> fo{infos};
        updateDescriptorSets(fo);

        return shared_from_this();
    }


    ImGuiDescriptorManager::ImGuiDescriptorManager() : ct(yic::systemHub.val<ev::pVkSetupContext>()),
                                                       activeImageIndex(yic::systemHub.val<ev::pVkRenderContext>().activeImageIndex) {
        ci.addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
        ci.buildDescriptorSetLayouts(ct.device);

        auto info = vk::SamplerCreateInfo()
                .setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                .setAddressModeW(vk::SamplerAddressMode::eRepeat);
        sampler = ct.device->createSampler(info);
    }

    auto ImGuiDescriptorManager::updateImage(const vot::string &id, const vot::vector<vk::ImageView> &views) -> void {
        mDescriptors[id] = std::make_shared<Descriptor>(ci);

        DescriptorLayout layout;
        for(auto& view : views){
            layout.emplace(DescriptorLayout::_1d {vk::DescriptorImageInfo{sampler, view, vk::ImageLayout::eShaderReadOnlyOptimal}});
        }

        mDescriptors[id]->updateDescriptorSets(layout);
    }

    auto ImGuiDescriptorManager::updateDepthImage(const vot::string &id, const vk::ImageView &view) -> void {

    }

    auto ImGuiDescriptorManager::drawImage(const vot::string &id, const ImVec2 &imageSize) -> void {
        if (!mDescriptors[id])
            return;
        auto& descriptors = mDescriptors[id]->acquire();

        if (*activeImageIndex < 0 || *activeImageIndex == UINT32_MAX){
            for(const auto& des : descriptors){
                 ImGui::Image((ImTextureID)des, imageSize);
            }
        } else {
            auto e = vot::Resolutions::eQHDExtent;
            auto windowSize = ImGui::GetWindowSize();

            float scaleX = (float) windowSize.x / (float) e.width;
            float scaleY = (float) windowSize.y / (float) e.height;
            float scale = std::min(scaleX, scaleY);

            auto scaledWidth = (float) e.width * scale;
            auto scaledHeight = (float) e.height * scale;

            float cursorX = (windowSize.x - scaledWidth) * 0.5f;
            float cursorY = (windowSize.y - scaledHeight + 40.f) * 0.5f;

            ImGui::SetCursorPos({cursorX > 0 ? cursorX : 0, cursorY > 0 ? cursorY : 0});

            if (!descriptors.empty())
                ImGui::Image(descriptors.size() > *activeImageIndex ? (ImTextureID)descriptors[*activeImageIndex] : (ImTextureID)descriptors.back(), ImVec2{scaledWidth, scaledHeight});
        }
    }
} // rhi