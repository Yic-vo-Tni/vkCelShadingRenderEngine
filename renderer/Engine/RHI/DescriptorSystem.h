#include <utility>

//
// Created by lenovo on 6/5/2025.
//

#ifndef VKCELSHADINGRENDERER_DESCRIPTORSYSTEM_H
#define VKCELSHADINGRENDERER_DESCRIPTORSYSTEM_H

#include "Pipeline/GraphicsPipeline.h"
#include "Pipeline/RayTracingPipeline.h"

namespace rhi {

    class DescriptorSystem {
    public:
        using descriptorInfo = std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR>;
        using descVec1D = vot::vector<descriptorInfo>;
        using descVec2D = vot::vector<descVec1D>;
        using descVec3D = vot::vector<descVec2D>;
    public:
        Make = []{ return Singleton<DescriptorSystem>::make_ptr();};
        DescriptorSystem();
        ~DescriptorSystem() = default;

        auto allocUpdateDescriptorSets(const vot::vector<vot::vector<vot::vector<descriptorInfo>>>& infos, vot::PipelineDescriptorSetLayoutCI2 ci2, const uint32_t& startIndex, const uint32_t &sliceCount) -> vot::DescriptorHandle;
        auto allocUpdateDescriptorSets(const vot::vector<vot::vector<descriptorInfo>>& infos, vot::PipelineDescriptorSetLayoutCI2 ci2, const uint32_t& startIndex, const uint32_t &sliceCount) -> vot::DescriptorHandle;
        auto allocUpdateDescriptorSets(const vot::vector<descriptorInfo>& infos, vot::PipelineDescriptorSetLayoutCI2 ci2, const uint32_t& startIndex, const uint32_t &sliceCount) -> vot::DescriptorHandle ;
        auto allocUpdateDescriptorSets(const std::function<vot::DescriptorLayout2()>& layout, const vot::PipelineDescriptorSetLayoutCI2& ci2, const uint32_t& startIndex = 1, const uint32_t &sliceCount = 0)-> vot::DescriptorHandle  {
            return allocUpdateDescriptorSets(layout(), ci2, startIndex, sliceCount);
        };
        auto allocUpdateDescriptorSets(const std::function<vot::DescriptorLayout2()>& layout, GraphicsPipeline& graphicsPipeline, const uint32_t& startIndex = 1, const uint32_t &sliceCount = 0)-> vot::DescriptorHandle  {
            return allocUpdateDescriptorSets(layout(), std::get<vot::PipelineDescriptorSetLayoutCI2>(graphicsPipeline.acquirePipelineLibrary().pipelineDescriptorSetLayoutCI), startIndex, sliceCount);
        };
        auto allocUpdateDescriptorSets(const std::function<vot::DescriptorLayout2()>& layout, RayTracingPipeline& rayTracingPipeline, const uint32_t& startIndex = 1, const uint32_t &sliceCount = 0)-> vot::DescriptorHandle  {
            return allocUpdateDescriptorSets(layout(), rayTracingPipeline.getDescriptorSetLayoutCI2(), startIndex, sliceCount);
        };
        auto updateDescriptorSets( const vot::vector<vot::vector<vot::vector<DescriptorSystem::descriptorInfo>>> &infos, vot::PipelineDescriptorSetLayoutCI2 ci2, vot::DescriptorHandle& handle) -> void;
        auto updateDescriptorSets(const vot::vector<vot::vector<descriptorInfo>>& infos, vot::PipelineDescriptorSetLayoutCI2 ci2, vot::DescriptorHandle& handle) -> void;
        auto updateDescriptorSets(const vot::vector<descriptorInfo>& infos, vot::PipelineDescriptorSetLayoutCI2 ci2, vot::DescriptorHandle& handle) -> void ;
        auto updateDescriptorSets(const std::function<vot::DescriptorLayout2()>& layout, const vot::PipelineDescriptorSetLayoutCI2& ci2, vot::DescriptorHandle& handle)-> void { updateDescriptorSets(layout(), ci2, handle); };
        auto updateDescriptorSets(const std::function<vot::DescriptorLayout2()>& layout, GraphicsPipeline& graphicsPipeline, vot::DescriptorHandle& handle) -> void {
            updateDescriptorSets(layout(), std::get<vot::PipelineDescriptorSetLayoutCI2>(graphicsPipeline.acquirePipelineLibrary().pipelineDescriptorSetLayoutCI), handle);
        };
        auto updateDescriptorSets(const std::function<vot::DescriptorLayout2()>& layout, RayTracingPipeline& rayTracingPipeline, vot::DescriptorHandle& handle) -> void {
            updateDescriptorSets(layout(), rayTracingPipeline.getDescriptorSetLayoutCI2(), handle);
        };

        auto clear() { ct.device->destroy(mDescriptorPool); }
    private:
        auto buildDescriptorPool(const uint32_t& maxSets = 1000) -> void;
        auto pushbackDescriptorHandle(const vot::PipelineDescriptorSetLayoutCI2& ci2, const uint32_t& startIndex, vot::DescriptorHandle& handle, const uint32_t &sliceCount) -> void;

        ev::pVkSetupContext ct;

        vk::DescriptorPool mDescriptorPool;
        vot::vector<vk::DescriptorSet> mDescriptorSets;
    };

} // rhi


namespace yic{
    inline rhi::DescriptorSystem* desSystem;
}

#endif //VKCELSHADINGRENDERER_DESCRIPTORSYSTEM_H
