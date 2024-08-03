//
// Created by lenovo on 7/8/2024.
//

#ifndef VKCELSHADINGRENDERER_DESCRIPTOR_H
#define VKCELSHADINGRENDERER_DESCRIPTOR_H

#include <utility>

#include "Engine/RHI/vkCommon.h"
#include "Engine/ECS/Model/ModelStruct.h"

#include "Engine/Utils/Log.h"

namespace yic {

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
        explicit ImgInfo(const sc::Model& model){
            for(const auto& mesh : model.meshes){
                imageViews.emplace_back(model.diffTexs[mesh.texIndex]->imageViews.back());
            }
        }
        explicit ImgInfo(const std::vector<std::shared_ptr<yic::vkImage>>& imgs){
            for(const auto& img : imgs){
                imageViews.emplace_back(img->imageViews.back());
            }
        }
    };
    struct BufInfo{
        std::vector<vk::Buffer> buffer;
        std::vector<vk::DeviceSize> offset{0};
        std::vector<vk::DeviceSize> range{VK_WHOLE_SIZE};

        explicit BufInfo(vk::Buffer buffer) : buffer(std::vector<vk::Buffer>{buffer}){}
        explicit BufInfo(const std::vector<vk::Buffer>& buffer) : buffer(buffer){}
    };

    class Descriptor : public Identifiable{
    public:
        explicit Descriptor(const std::string& id, PipelineDesSetLayout& setLayout);
        ~Descriptor() override{
            if (mDesPool) {
                mDevice.destroyDescriptorPool(mDesPool);
            }
        }

        [[nodiscard]] inline auto& getDescriptorSets() const { return mDesSet;}

        Descriptor& createDesPool(std::optional<uint32_t> Reset_MaxSets = std::nullopt);
        Descriptor& pushbackDesSets(uint32_t setIndex = 0);
        Descriptor& updateDesSet(const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &infos, const size_t& setIndex = 0);
        Descriptor& updateDesSet(uint32_t Reset_MaxSets, const std::vector<std::variant<ImgInfo, BufInfo>> &infos, const size_t& setIndex = 0);
        Descriptor& updateDesSet(const std::vector<std::variant<ImgInfo, BufInfo>> &infos, const size_t& setIndex = 0){
            updateDesSet(1, infos, setIndex);
            return *this;
        };

        auto getSetLayout() { return mSetLayout; }

    private:
        vk::Device mDevice;

        uint32_t mIndex{};
        vk::DescriptorPool mDesPool{};
        PipelineDesSetLayout& mSetLayout;
        std::vector<vk::DescriptorSet> mDesSet{};
        std::vector<std::vector<vk::WriteDescriptorSet>> mWriteDesSets{};
    };

    class ImGuiDescriptorManager{
    public:
        vkGet auto get = []{ return Singleton<ImGuiDescriptorManager>::get(); };
        ImGuiDescriptorManager();
        ~ImGuiDescriptorManager();

        static auto updateImage(const std::string& id, const std::vector<vk::ImageView>& views) -> void;
        static auto drawImage(const std::string& id, const ImVec2& imageSize, const uint32_t& index = UINT32_MAX) -> void;
        static auto clear() { get()->mSetLayout.reset(); }
    private:
        std::shared_ptr<PipelineDesSetLayout> mSetLayout;
    };

} // yic

#endif //VKCELSHADINGRENDERER_DESCRIPTOR_H
