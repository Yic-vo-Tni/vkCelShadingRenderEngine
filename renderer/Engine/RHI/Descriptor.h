//
// Created by lenovo on 10/3/2024.
//

#ifndef VKCELSHADINGRENDERER_DESCRIPTOR_H
#define VKCELSHADINGRENDERER_DESCRIPTOR_H

#include "RHI/Pipeline/RayTracingPipeline.h"

namespace rhi {

    class Descriptor : public std::enable_shared_from_this<Descriptor>{
        using descriptorInfo = std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR>;
        using descVec1D = vot::vector<descriptorInfo>;
        using descVec2D = vot::vector<descVec1D>;
        using descVec3D = vot::vector<descVec2D>;
    public:
        struct Layout{
            using _1d = vot::vector<descriptorInfo>;
            using _2d = vot::vector<descVec1D>;
            using _3d = vot::vector<descVec2D>;

            auto emplace(const descVec3D& layout){
                infos = layout;
                return *this;
            }
            auto emplace(const descVec2D& layout){
                infos.emplace_back(layout);
                return *this;
            }
            auto emplace(const descVec1D& bindings){
                infos.emplace_back(descVec2D{bindings});
                return *this;
            }

            operator vot::vector<vot::vector<vot::vector<descriptorInfo>>>(){
                return std::move(infos);
            }
        private:
            vot::vector<vot::vector<vot::vector<descriptorInfo>>> infos;
        };

        Descriptor();
        explicit Descriptor(vot::PipelineDescriptorSetLayoutCI  pipelineDescriptorSetLayoutCi);
        ~Descriptor();

        static auto make_shared() { return std::make_shared<Descriptor>(); }
        auto combine(const vot::PipelineDescriptorSetLayoutCI& ci) { mPipelineDescriptorSetLayoutCI = ci; return shared_from_this(); }
//        auto combine(const vot::PipelineLibrary& ci) { mPipelineDescriptorSetLayoutCI = ci.pipelineDescriptorSetLayoutCI; return shared_from_this(); }
        auto combine(const vot::PipelineLibrary& ci) { mPipelineDescriptorSetLayoutCI = std::get<vot::PipelineDescriptorSetLayoutCI>(ci.pipelineDescriptorSetLayoutCI); return shared_from_this(); }
        auto combine(rhi::RayTracingPipeline& rayTracingPipeline) { mPipelineDescriptorSetLayoutCI = rayTracingPipeline.getDescriptorSetLayoutCI() ; return shared_from_this(); }
        auto updateDescriptorSets(const vot::vector<vot::vector<vot::vector<descriptorInfo>>>& infos) -> std::shared_ptr<Descriptor>;
        auto updateDescriptorSets(const vot::vector<vot::vector<descriptorInfo>>& infos) -> std::shared_ptr<Descriptor>;
        auto updateDescriptorSets(const vot::vector<descriptorInfo>& infos) -> std::shared_ptr<Descriptor>;
        auto updateDescriptorSets(const std::function<Layout()>& layout) -> std::shared_ptr<Descriptor>{ updateDescriptorSets(layout()); return shared_from_this();};

        auto& acquire() { return mDescriptorSets; }
    private:
        auto createDescriptorPool(const std::optional<uint32_t>& resetMaxSets = std::nullopt) -> void;
        auto pushbackDescriptorSets() -> void;
    private:
        ev::pVkSetupContext ct{};

        uint8_t mIndex{0};
        vk::DescriptorPool mDescriptorPool{};
        vot::vector<vk::DescriptorSet> mDescriptorSets;
        vot::vector<vot::vector<vk::WriteDescriptorSet>> mWriteDescriptorSets{};
        vot::PipelineDescriptorSetLayoutCI mPipelineDescriptorSetLayoutCI;
    };

    using DescriptorLayout = Descriptor::Layout;


    class ImGuiDescriptorManager{
    public:
        Make = []{ return Singleton<ImGuiDescriptorManager>::make_ptr(); };
        ImGuiDescriptorManager();
        ~ImGuiDescriptorManager() = default;

        auto updateImage(const vot::string& id, const vot::vector<vk::ImageView>& views) -> void;
        auto updateDepthImage(const vot::string& id, const vk::ImageView& view) -> void;
        auto drawImage(const vot::string& id, const ImVec2& imageSize = ImGui::GetContentRegionAvail()) -> void;
        //auto clear() -> void{ for(auto& desSetLayout : ci.desSetLayouts) { ct.device->destroy(desSetLayout); }}
        auto clear() -> void{ mDescriptors.clear(); ci.clear(); ct.device->destroy(sampler); }

        inline static vk::Sampler sampler = nullptr;
    private:
        ev::pVkSetupContext ct{};
        uint32_t* activeImageIndex{};
        vot::PipelineDescriptorSetLayoutCI ci;
        std::unordered_map<vot::string, std::shared_ptr<Descriptor>> mDescriptors;
    };

} // rhi

namespace yic{
    inline rhi::ImGuiDescriptorManager* imguiImage;
}


#endif //VKCELSHADINGRENDERER_DESCRIPTOR_H
