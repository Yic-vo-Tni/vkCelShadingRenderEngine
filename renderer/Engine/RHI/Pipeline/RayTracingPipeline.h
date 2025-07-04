//
// Created by lenovo on 11/24/2024.
//

#ifndef VKCELSHADINGRENDERER_RAYTRACINGPIPELINE_H
#define VKCELSHADINGRENDERER_RAYTRACINGPIPELINE_H

namespace rhi {

    class RayTracingPipeline {
        struct ReBuild{
            const vot::string path;
            const vk::ShaderStageFlagBits flags;
            const vk::RayTracingShaderGroupTypeKHR type;
            const vot::RTShaderRole role;
        };
    public:
        vot::DescriptorHandle DS;
    public:
        RayTracingPipeline();
        ~RayTracingPipeline();

        auto
        addShader(vot::string path, const vk::ShaderStageFlagBits &flags,
                  const vk::RayTracingShaderGroupTypeKHR &type,
                  const vot::RTShaderRole &role = vot::RTShaderRole::eGeneral) -> RayTracingPipeline&;
        auto build(const std::variant<vot::PipelineDescriptorSetLayoutCI, vot::PipelineDescriptorSetLayoutCI2>& ci) ->RayTracingPipeline&;
        auto build() -> void;
//        auto& getDescriptorSetLayoutCI() { return mDesSetLayoutCI; }
        auto& getDescriptorSetLayoutCI() { return std::get<vot::PipelineDescriptorSetLayoutCI>(mDesSetLayoutCI); }
        auto& getDescriptorSetLayoutCI2(){ return std::get<vot::PipelineDescriptorSetLayoutCI2>(mDesSetLayoutCI); }
        auto& acquire() { return mRTPipeline; }
        auto& acquirePipelineLayout() { return mRTPipelineLayout; }
        auto& gRgen() { return mRgenRegion; }
        auto& gMiss() { return mMissRegion; }
        auto& gHit() { return mHitRegion; }
        auto& gCall() { return mCallRegion; }
        auto acquirePipelineBindPoint() { return vk::PipelineBindPoint::eRayTracingKHR; }

        auto& getRebuilds() { return mRebuilds; }
        auto clear() -> void{
            mRgenCount = 0; mMissCount = 0;
            mHitCount = 0; mCallCount = 0;
            mShaderStages.clear(); mShaderGroups.clear();
        }
    private:
        auto cSBT() -> void;

    private:
        ev::pVkSetupContext ct;
        vk::Pipeline mRTPipeline;
        vk::PipelineLayout mRTPipelineLayout;
//        vot::PipelineDescriptorSetLayoutCI mDesSetLayoutCI;
        std::variant<vot::PipelineDescriptorSetLayoutCI, vot::PipelineDescriptorSetLayoutCI2> mDesSetLayoutCI;
        vot::vector<ReBuild> mRebuilds;

        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR mRTProperties;

        vot::vector<vk::PipelineShaderStageCreateInfo> mShaderStages{};
        vot::vector<vk::RayTracingShaderGroupCreateInfoKHR> mShaderGroups{};

        vot::Buffer_sptr mSBTBuffer{};
        uint8_t mRgenCount{0}, mMissCount{0}, mHitCount{0}, mCallCount{0};
        vk::StridedDeviceAddressRegionKHR mRgenRegion{}, mMissRegion{}, mHitRegion{}, mCallRegion{};

        vot::vector<vot::string> mShaderPaths{};
        vot::Buffer_sptr mRgenSBT, mMissSBT, mRchitSBT;

        vot::vector<vk::ShaderModule> mShaderModules;
    };

} // rhi

#endif //VKCELSHADINGRENDERER_RAYTRACINGPIPELINE_H
