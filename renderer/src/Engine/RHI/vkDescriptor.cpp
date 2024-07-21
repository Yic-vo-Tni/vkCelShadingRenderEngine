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

    vkDescriptor &vkDescriptor::updateDesSet(uint32_t Reset_MaxSets,
                                             const std::vector<std::variant<ImgInfo, BufInfo>> &infos,
                                             const size_t &setIndex) {
        createDesPool(Reset_MaxSets);

        for (int i = 0; i < Reset_MaxSets; i++) {
            std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> v;
            for (auto &info: infos) {
                std::visit([&](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<T, BufInfo>) {
                        v.emplace_back(vk::DescriptorBufferInfo{arg.buffer[i] ? arg.buffer[i] : arg.buffer.back(),
                                                                arg.offset[i] ? arg.offset[i] : arg.offset.back(),
                                                                arg.range[i] ? arg.range[i] : arg.range.back()});
                    } else if constexpr (std::is_same_v<T, ImgInfo>) {
                        v.emplace_back(vk::DescriptorImageInfo{arg.sampler,
                                                               arg.imageViews[i] ? arg.imageViews[i] : arg.imageViews.back(),
                                                               arg.imageLayout});
                    }
                }, info);
            }

            updateDesSet(v, setIndex);
        }

        return *this;
    }

    ///---------------------------------------------------------------------------------------------------------------------///

    vk::Sampler vkDescriptor::FixSampler::eDefault;

    vkDescriptor::FixSampler::FixSampler() {
        auto dev = EventBus::Get::vkSetupContext().device_ref();

        eDefault = dev.createSampler(vk::SamplerCreateInfo{
                {},
                vk::Filter::eLinear, vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                0.f, vk::False, 1.f,
                vk::False, vk::CompareOp::eAlways, 0.f, 0.f,
                vk::BorderColor::eIntOpaqueBlack, vk::False
        });
    }

    vkDescriptor::FixSampler::~FixSampler() {
        auto dev = EventBus::Get::vkSetupContext().device_ref();
        dev.destroy(eDefault);
    }

    auto vkDescriptor::FixSampler::createSampler() -> vk::Sampler {
        auto dev = EventBus::Get::vkSetupContext().device_ref();

        return dev.createSampler(vk::SamplerCreateInfo{
                {},
                vk::Filter::eLinear, vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                0.f, vk::False, 1.f,
                vk::False, vk::CompareOp::eAlways, 0.f, 0.f,
                vk::BorderColor::eIntOpaqueBlack, vk::False
        });
    }

    auto ImGuiDescriptorManager::updateImage(const std::string &id, const std::vector<vk::ImageView> &views) -> void {
        auto desc = std::make_shared<vkDescriptor>(id);
        EventBus::update(et::vkResource{
            .desc = desc
        });

        desc->addDesSetLayout({vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1,
                                                              vk::ShaderStageFlagBits::eFragment}});
        desc->updateDesSet(views.size(), {vkDescriptor::ImgInfo{views}});
    }

    auto ImGuiDescriptorManager::drawImage(const std::string &id, const ImVec2& imageSize, const uint32_t& index) -> void {
        if (!EventBus::Get::vkResource().desc_exists())
            return;

        if (EventBus::Get::vkResource().desc_ref().find_ref(id) == nullptr){
            return;
        }
        auto& descs = EventBus::Get::vkResource().desc_ref().find_ref(id)->getDescriptorSets();

        if (index == UINT32_MAX){
            for(auto& desc : descs){
                ImGui::Image((ImTextureID)desc, imageSize);
            }
        } else{
            ImGui::Image((ImTextureID)descs[index], imageSize);
        }

    }

//    auto ImGuiDescriptorManager::drawImage(const std::string &id, const std::vector<vk::ImageView> &views,
//                                           const ImVec2 &imageSize, const int &index) -> void {
//        updateImage(id, views);
//        drawImage(id, imageSize, index);
//    }


} // yic