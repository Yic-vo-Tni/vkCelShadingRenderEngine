//
// Created by lenovo on 8/13/2024.
//

#ifndef VKCELSHADINGRENDERER_RAYTRACINGPIPELINE_H
#define VKCELSHADINGRENDERER_RAYTRACINGPIPELINE_H

#include "State.h"
#include "Engine/RHI/vkCommon.h"

namespace yic {

    class RayTracing : public State<RayTracing>{
    private:
        struct ShaderModule{
            vk::ShaderModule shaderModule{};
            vk::ShaderStageFlagBits flags{};
        };
        std::unordered_map<vk::ShaderStageFlagBits, vk::RayTracingShaderGroupTypeKHR> shaderTypeMap = {
                {vk::ShaderStageFlagBits::eClosestHitKHR, vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup},
                {vk::ShaderStageFlagBits::eRaygenKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral},
                {vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral}
        };
    public:
        RayTracing();
        ~RayTracing();

        auto create() -> void;

        auto addShader(std::string path, vk::ShaderStageFlagBits flags) -> void;

        void createRtSBT();

        auto createShaderStage() -> void;

        auto createPipeline() -> void;

        [[nodiscard]] inline auto& acquire() const { return mRTPipeline;}
        [[nodiscard]] inline auto getShaderPaths() const { return mShaderPaths;}
        [[nodiscard]] inline auto& getRegionRgen() const { return regionRgen;}
        [[nodiscard]] inline auto& getRegionMiss() const { return regionMiss;}
        [[nodiscard]] inline auto& getRegionHit() const { return regionHit;}
        [[nodiscard]] inline auto& getRegionCall() const { return regionCall;}
    private:

    protected:
        vk::Device mDevice{};
        vk::DispatchLoaderDynamic mDyDispatcher{};
        vk::PipelineLayout mPipelineLayout{};
        vk::Pipeline mRTPipeline{};
    public:
        vk::RayTracingPipelineCreateInfoKHR rtCreateInfo{};
    protected:
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties;

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> rtShaderGroups;
//        std::unordered_map<std::string, ShaderModule> shaderModules{};

        std::shared_ptr<vkBuffer> sbtBuf{};
        uint8_t rgenCount{0}, missCount{0}, hitCount{0}, callCount{0};
        vk::StridedDeviceAddressRegionKHR regionRgen{}, regionMiss{}, regionHit{}, regionCall{};

        std::vector<std::string> mShaderPaths;

        std::shared_ptr<vkBuffer> rgenSbt, rmissSbt, rchitSbt;


        vot::vector<std::pair<std::string, ShaderModule>> shaderModules{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_RAYTRACINGPIPELINE_H
