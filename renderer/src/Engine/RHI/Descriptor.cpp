//
// Created by lenovo on 7/8/2024.
//

#include "Descriptor.h"

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    Descriptor::Descriptor(const std::string &id, PipelineDesSetLayout& setLayout) :
            Identifiable(id), mSetLayout(setLayout), mDevice(EventBus::Get::vkSetupContext().device_ref()) {

    }

    Descriptor &Descriptor::createDesPool(std::optional<uint32_t> Reset_MaxSets) {
        if (Reset_MaxSets.has_value()){
            mSetLayout.maxSets = Reset_MaxSets.value();
        }

        if (mSetLayout.desSetLayouts.empty()) {
            for (auto &bindings: mSetLayout.bindings) {
                vk::DescriptorSetLayoutCreateInfo createInfo{{}, bindings};

                mSetLayout.desSetLayouts.emplace_back(
                        vkCreate("create descriptor set layout") = [&] {
                            return mDevice.createDescriptorSetLayout(createInfo);
                        });
            }
        }

        vk::DescriptorPoolCreateInfo createInfo{
            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            mSetLayout.maxSets, mSetLayout.poolSize
        };

        mDesPool = vkCreate("create des pool") = [&]{
            return mDevice.createDescriptorPool(createInfo);
        };

        return *this;
    }

    Descriptor &Descriptor::pushbackDesSets(uint32_t setIndex) {
        vk::DescriptorSetAllocateInfo allocateInfo{mDesPool, mSetLayout.desSetLayouts};

        auto des = vkCreate("create des" + std::to_string(setIndex) + "set", spdlog::level::trace) = [&]{
            return mDevice.allocateDescriptorSets(allocateInfo)[setIndex];
        };
        //mDesSet.insert(mDesSet.end(), des.begin(), des.end());
        mDesSet.emplace_back(des);

        return *this;
    }

    Descriptor &Descriptor::updateDesSet(
            const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR>> &infos, const size_t& setIndex) {
        pushbackDesSets();

        mWriteDesSets.resize(mIndex + 1);
        for(uint32_t i = 0; auto& bind : mSetLayout.bindings[setIndex]){
            std::visit([&](auto&& arg){
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>){
                    mWriteDesSets[mIndex].push_back(vk::WriteDescriptorSet{mDesSet[mIndex], bind.binding, 0, bind.descriptorType, {}, arg});
                } else if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>){
                    mWriteDesSets[mIndex].push_back(vk::WriteDescriptorSet{mDesSet[mIndex], bind.binding, 0, bind.descriptorType, arg});
                } else if constexpr (std::is_same_v<T, vk::WriteDescriptorSetAccelerationStructureKHR>){
                    mWriteDesSets[mIndex].push_back(vk::WriteDescriptorSet{mDesSet[mIndex], bind.binding, 0, 1, bind.descriptorType, {}, {}, {}, &arg});
                }

            }, infos[i]);
            i++;
        }

        mDevice.updateDescriptorSets(mWriteDesSets[mIndex], nullptr);

        mIndex++;

        return *this;
    }

    Descriptor &Descriptor::updateDesSet(uint32_t Reset_MaxSets,
                                             const std::vector<std::variant<ImgInfo, BufInfo, AccelInfo>> &infos,
                                             const size_t &setIndex) {
        if (!mDesPool){
            createDesPool(Reset_MaxSets);
        } else{
            mDevice.resetDescriptorPool(mDesPool);
        }

        mDesSet.clear();
        mWriteDesSets.clear();
        mIndex = 0;

        for (int i = 0; i < Reset_MaxSets; i++) {
            std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR>> v;
            for (auto &info: infos) {
                std::visit([&](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<T, BufInfo>) {
                        vk::Buffer buffer = (i < arg.buffer.size()) ? arg.buffer[i] : arg.buffer.back();
                        vk::DeviceSize offset = (i < arg.offset.size()) ? arg.offset[i] : arg.offset.back();
                        vk::DeviceSize range = (i < arg.range.size()) ? arg.range[i] : arg.range.back();

                        v.emplace_back(vk::DescriptorBufferInfo{buffer, offset, range});
                    } else if constexpr (std::is_same_v<T, ImgInfo>) {
                        vk::ImageView imageView = (i < arg.imageViews.size()) ? arg.imageViews[i] : arg.imageViews.back();

                        v.emplace_back(vk::DescriptorImageInfo{arg.sampler, imageView, arg.imageLayout});
                    } else if constexpr (std::is_same_v<T, AccelInfo>){
                        v.emplace_back(vk::WriteDescriptorSetAccelerationStructureKHR{arg.accel});
                    }
                }, info);
            }

            updateDesSet(v, setIndex);
        }

        return *this;
    }

    ///---------------------------------------------------------------------------------------------------------------------///

    vk::Sampler FixSampler::eDefault;

    FixSampler::FixSampler() {
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

    FixSampler::~FixSampler() {
        auto dev = EventBus::Get::vkSetupContext().device_ref();
        dev.destroy(eDefault);
    }

    auto FixSampler::createSampler() -> vk::Sampler {
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
        auto setLayout = get()->mSetLayout;

        auto desc = std::make_shared<Descriptor>(id, *setLayout);
        EventBus::update(et::vkResource{
            .desc = desc
        });

//        desc->addDesSetLayout({vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1,
//                                                              vk::ShaderStageFlagBits::eFragment}});
        desc->updateDesSet(views.size(), {ImgInfo{views}});
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
            ImGui::Image((ImTextureID)descs[index] ? (ImTextureID)descs[index] : (ImTextureID)descs.back(), imageSize);
        }

    }

    ImGuiDescriptorManager::ImGuiDescriptorManager() {
        mSetLayout = std::make_shared<PipelineDesSetLayout>(EventBus::Get::vkSetupContext().device_ref());

        mSetLayout->addDesSetLayout(0, 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
        mSetLayout->createDesSetLayout();
    }
    ImGuiDescriptorManager::~ImGuiDescriptorManager() {
        mSetLayout.reset();
    }


} // yic