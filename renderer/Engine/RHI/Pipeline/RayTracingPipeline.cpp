//
// Created by lenovo on 11/24/2024.
//

#include "RayTracingPipeline.h"
#include "Utils/FileOperation.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"

namespace rhi {

    template<class integral>
    constexpr integral align_up(integral x, size_t a) noexcept{
        return integral((x + (integral(a) - 1)) & ~integral(a - 1));
    }


    RayTracingPipeline::RayTracingPipeline() : ct(yic::systemHub.val<ev::pVkSetupContext>()) {
        vk::PhysicalDeviceProperties2 properties2{};
        properties2.pNext = &mRTProperties;
        ct.physicalDevice->getProperties2(&properties2);
    }

    RayTracingPipeline::~RayTracingPipeline() {
//        mDesSetLayoutCI.clear();

        std::visit([&](auto&& arg){
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI>){
                arg.clear();
            } else if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI2>){
                arg.clear(ct.device);
            }
        }, mDesSetLayoutCI);

        ct.device->destroy(mRTPipelineLayout);
        ct.device->destroy(mRTPipeline);
    }

    auto RayTracingPipeline::addShader(vot::string path, const vk::ShaderStageFlagBits &flags,
                                       const vk::RayTracingShaderGroupTypeKHR &type,
                                       const vot::RTShaderRole &role) -> RayTracingPipeline& {
        mRebuilds.emplace_back(ReBuild{path, flags, type, role});
        yic::shaderHot->rego(path, {.rp = this, .flags = flags});

        path = spv_path + path + ".spv";
        vot::vector<char> v;
        std::ranges::copy(fo::loadFile(path), std::back_inserter(v));
        auto sm = ct.device->createShaderModule(vk::ShaderModuleCreateInfo()
                                                              .setCodeSize(v.size() * sizeof(char))
                                                              .setPCode(reinterpret_cast<const uint32_t *>(v.data())));
        switch (flags) {
            case vk::ShaderStageFlagBits::eRaygenKHR:
                mRgenCount++;
                break;
            case vk::ShaderStageFlagBits::eMissKHR:
                mMissCount++;
                break;
            case vk::ShaderStageFlagBits::eClosestHitKHR:
                mHitCount++;
                break;
            default:
                break;
        }

        mShaderStages.emplace_back(vk::PipelineShaderStageCreateInfo()
                                           .setModule(sm)
                                           .setPName("main")
                                           .setStage(flags));
        mShaderGroups.emplace_back(vk::RayTracingShaderGroupCreateInfoKHR()
                                           .setType(type)
                                           .setGeneralShader(role == vot::RTShaderRole::eGeneral ? mShaderStages.size() - 1 : vk::ShaderUnusedKHR)
                                           .setClosestHitShader(role == vot::RTShaderRole::eClosestHit ? mShaderStages.size() - 1 : vk::ShaderUnusedKHR)
                                           .setAnyHitShader(role == vot::RTShaderRole::eAnyHit ? mShaderStages.size() - 1 : vk::ShaderUnusedKHR)
                                           .setIntersectionShader(role == vot::RTShaderRole::eIntersection ? mShaderStages.size() - 1 : vk::ShaderUnusedKHR));
        mShaderModules.emplace_back(sm);

        return *this;
    }

    auto RayTracingPipeline::cSBT() -> void{
        const uint32_t hs = mRTProperties.shaderGroupHandleSize;
        const uint32_t hsAligned = align_up(mRTProperties.shaderGroupHandleSize, mRTProperties.shaderGroupHandleAlignment);
        const uint32_t numGroup = mShaderGroups.size();
        const uint32_t sbtSize = numGroup * hsAligned;

        auto handles = ct.device->getRayTracingShaderGroupHandlesKHR<uint8_t >(mRTPipeline, 0, numGroup, sbtSize, *ct.dynamicDispatcher);
        auto bufUsage = vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;

        mRgenSBT = yic::allocator->allocBuffer(hs, handles.data(), bufUsage, "rgen sbt");
        mMissSBT = yic::allocator->allocBuffer(hs * mMissCount, handles.data() + hsAligned * mRgenCount, bufUsage, "rmiss sbt");
        mRchitSBT = yic::allocator->allocBuffer(hs * mHitCount, handles.data() + hsAligned * (mRgenCount + mMissCount), bufUsage, "rchit sbt");

        mRgenRegion = vk::StridedDeviceAddressRegionKHR()
                .setDeviceAddress(mRgenSBT->bufferAddr())
                .setSize(hsAligned)
                .setStride(hsAligned);
        mMissRegion = vk::StridedDeviceAddressRegionKHR()
                .setDeviceAddress(mMissSBT->bufferAddr())
                .setStride(hsAligned)
                .setSize(hsAligned * mMissCount);
        mHitRegion = vk::StridedDeviceAddressRegionKHR()
                .setDeviceAddress(mRchitSBT->bufferAddr())
                .setStride(hsAligned)
                .setSize(hsAligned * mHitCount);
        mCallRegion = vk::StridedDeviceAddressRegionKHR();
    }

    auto RayTracingPipeline::build(const std::variant<vot::PipelineDescriptorSetLayoutCI, vot::PipelineDescriptorSetLayoutCI2>& ci) -> RayTracingPipeline& {
        if (mRTPipelineLayout) ct.device->destroy(mRTPipelineLayout);
        if (mRTPipeline) ct.device->destroy(mRTPipeline);

        mDesSetLayoutCI = ci;
        //mRTPipelineLayout = mDesSetLayoutCI.buildPipelineSetLayout(ct.device);
        std::visit([&](auto&& arg){
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI>){
                mRTPipelineLayout = arg.buildPipelineSetLayout(ct.device);
            } else if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI2>){
                mRTPipelineLayout = arg.buildPipelineSetLayout(ct.device);
            }
        }, mDesSetLayoutCI);

        mRTPipeline = vot::create("rt pipeline") = [&]{
            return ct.device->createRayTracingPipelineKHR(nullptr, nullptr, vk::RayTracingPipelineCreateInfoKHR()
                    .setStages(mShaderStages)
                    .setGroups(mShaderGroups)
                    .setMaxPipelineRayRecursionDepth(1)
                    .setLayout(mRTPipelineLayout), nullptr, *ct.dynamicDispatcher).value;
        };

        cSBT();

        for(auto& sm : mShaderModules){
            ct.device->destroy(sm);
        }
        mShaderModules.clear();

        return *this;
    }

    auto RayTracingPipeline::build() -> void {
        build(mDesSetLayoutCI);
    }


} // rhi