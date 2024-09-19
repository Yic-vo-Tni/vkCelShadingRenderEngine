//
// Created by lenovo on 8/13/2024.
//

#include "RayTracingPipeline.h"
#include "Engine/RHI/Allocator.h"

namespace yic {
    template<class integral>
    constexpr integral align_up(integral x, size_t a) noexcept{
        return integral((x + (integral(a) - 1)) & ~integral(a - 1));
    }

    RayTracing::RayTracing(): mDevice(*mg::SystemHub.val<ev::pVkSetupContext>().device),
            mDyDispatcher(*mg::SystemHub.val<ev::pVkSetupContext>().dynamicDispatcher) {
        vk::PhysicalDeviceProperties2 properties2{};
        properties2.pNext = &rtProperties;
        mg::SystemHub.val<ev::pVkSetupContext>().physicalDevice->getProperties2(&properties2);
    }

    RayTracing::~RayTracing() {
        if (mPipelineLayout){
            mDevice.destroy(mPipelineLayout);
        }

        for(auto& m : shaderModules){
            mDevice.destroy(m.second.shaderModule);
        }
        mDevice.destroy(mRTPipeline);
    }

    void RayTracing::createRtSBT() {
        const uint32_t hs = rtProperties.shaderGroupHandleSize;
        const uint32_t hsAligned = align_up(rtProperties.shaderGroupHandleSize, rtProperties.shaderGroupHandleAlignment);
        const uint32_t numGroup = rtShaderGroups.size();
        const uint32_t sbtSize = numGroup * hsAligned;

        auto handles = mDevice.getRayTracingShaderGroupHandlesKHR<uint8_t >(mRTPipeline, 0, numGroup, sbtSize, mDyDispatcher);
        auto bufUsage = vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;

        rgenSbt = mg::Allocator->allocBuffer(hs, handles.data(), bufUsage, "rgen sbt");
        rmissSbt = mg::Allocator->allocBuffer(hs * missCount, handles.data() + hsAligned * rgenCount, bufUsage, "rmiss sbt");
        rchitSbt = mg::Allocator->allocBuffer(hs * hitCount, handles.data() + hsAligned * (rgenCount + missCount), bufUsage, "rchit sbt");

        regionRgen = vk::StridedDeviceAddressRegionKHR()
                .setDeviceAddress(mg::Allocator->getBufAddr(rgenSbt))
                .setSize(hsAligned)
                .setStride(hsAligned);
        regionMiss = vk::StridedDeviceAddressRegionKHR()
                .setDeviceAddress(mg::Allocator->getBufAddr(rmissSbt))
                .setStride(hsAligned)
                .setSize(hsAligned * missCount);
        regionHit = vk::StridedDeviceAddressRegionKHR()
                .setDeviceAddress(mg::Allocator->getBufAddr(rchitSbt))
                .setStride(hsAligned)
                .setSize(hsAligned * hitCount);
        regionCall = vk::StridedDeviceAddressRegionKHR();
    }

    auto RayTracing::addShader(std::string path, vk::ShaderStageFlagBits flags) -> void {
            mShaderPaths.emplace_back(path);

            std::regex pattern("(\\.[^\\.]+)$");
            std::string replace = ".spv";
            auto spvPath = std::regex_replace(path, pattern, replace);
            path = spv_path + std::string("/") + spvPath;
            auto t = [this, path, flags]{
                std::vector<char> v;
                std::ranges::copy(fo::loadFile(path), std::back_inserter(v));

//                if (shaderModules[path].shaderModule)
//                    mDevice.destroy(shaderModules[path].shaderModule);
//
//                shaderModules[path].shaderModule = mDevice.createShaderModule(vk::ShaderModuleCreateInfo().setCodeSize(sizeof (char) * v.size())
//                                                                                      .setPCode(reinterpret_cast<const uint32_t*>(v.data())));
//                shaderModules[path].flags = flags;
                bool find = false;
                for(auto& shaderModule : shaderModules){
                    if (shaderModule.first == path){
                        mDevice.destroy(shaderModule.second.shaderModule);
                        shaderModule.second.shaderModule = mDevice.createShaderModule(vk::ShaderModuleCreateInfo()
                                .setCodeSize(sizeof(char) * v.size())
                                .setPCode(reinterpret_cast<const uint32_t*>(v.data())));
                        shaderModule.second.flags = flags;
                        find = true;
                    }
                }

                if (!find) {
                    ShaderModule sm{
                            mDevice.createShaderModule(vk::ShaderModuleCreateInfo()
                                                               .setCodeSize(sizeof(char) * v.size())
                                                               .setPCode(reinterpret_cast<const uint32_t *>(v.data()))),
                            flags
                    };
                    shaderModules.emplace_back(std::make_pair(path, sm));
                }
            };

            ShaderHotReLoader::registerShaderFileTask(path, [this, t]{
                t();
                createPipeline();
                createRtSBT();
            });

            t();
    }

    auto RayTracing::create() -> void {
        createPipeline();
        createRtSBT();
    }

    auto RayTracing::createShaderStage() -> void {
        shaderStages.clear();
        rtShaderGroups.clear();

//        std::vector<std::pair<std::string, ShaderModule>> sortedModules;
//
//        for (auto& [p, m] : shaderModules) {
//            if (m.flags == vk::ShaderStageFlagBits::eRaygenKHR) {
//                sortedModules.emplace_back(p, m);
//            }
//        }
//
//        for (auto& [p, m] : shaderModules) {
//            if (m.flags == vk::ShaderStageFlagBits::eMissKHR) {
//                sortedModules.emplace_back(p, m);
//            }
//        }
//
//        for (auto& [p, m] : shaderModules) {
//            if (m.flags == vk::ShaderStageFlagBits::eClosestHitKHR) {
//                sortedModules.emplace_back(p, m);
//            }
//        }
//
//        for(auto& [p, m] : sortedModules){
//            vk::PipelineShaderStageCreateInfo shaderStage{{}, m.flags, m.shaderModule, "main"};
//            shaderStages.push_back(shaderStage);
//
//            if (shaderTypeMap.count(m.flags) > 0){
//                vk::RayTracingShaderGroupCreateInfoKHR groupInfo{};
//                groupInfo.setType(shaderTypeMap[m.flags]);
//
//                switch (m.flags) {
//                    case vk::ShaderStageFlagBits::eClosestHitKHR:
//                        groupInfo.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
//                        groupInfo.setClosestHitShader(shaderStages.size() - 1)
//                                .setGeneralShader(vk::ShaderUnusedKhr)
//                                .setAnyHitShader(vk::ShaderUnusedKhr)
//                                .setIntersectionShader(vk::ShaderUnusedKhr);
//                        hitCount ++;
//                        break;
//                    case vk::ShaderStageFlagBits::eRaygenKHR:
//                    case vk::ShaderStageFlagBits::eMissKHR:
//                        groupInfo.setGeneralShader(shaderStages.size() - 1)
//                                .setClosestHitShader(vk::ShaderUnusedKhr)
//                                .setAnyHitShader(vk::ShaderUnusedKhr)
//                                .setIntersectionShader(vk::ShaderUnusedKhr);
//                        if (m.flags == vk::ShaderStageFlagBits::eRaygenKHR){
//                            rgenCount++;
//                        } else {
//                            missCount++;
//                        }
//                        break;
//                    default:
//                        throw std::runtime_error("");
//                }
//                rtShaderGroups.emplace_back(groupInfo);
//            }
//        }
        for(auto& [p, m] : shaderModules) {
            vk::PipelineShaderStageCreateInfo shaderStage{{}, m.flags, m.shaderModule, "main"};
            shaderStages.push_back(shaderStage);

            if (shaderTypeMap.count(m.flags) > 0) {
                vk::RayTracingShaderGroupCreateInfoKHR groupInfo{};
                groupInfo.setType(shaderTypeMap[m.flags]);

                switch (m.flags) {
                    case vk::ShaderStageFlagBits::eClosestHitKHR:
                        groupInfo.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
                        groupInfo.setClosestHitShader(shaderStages.size() - 1)
                                .setGeneralShader(vk::ShaderUnusedKhr)
                                .setAnyHitShader(vk::ShaderUnusedKhr)
                                .setIntersectionShader(vk::ShaderUnusedKhr);
                        hitCount++;
                        break;
                    case vk::ShaderStageFlagBits::eRaygenKHR:
                    case vk::ShaderStageFlagBits::eMissKHR:
                        groupInfo.setGeneralShader(shaderStages.size() - 1)
                                .setClosestHitShader(vk::ShaderUnusedKhr)
                                .setAnyHitShader(vk::ShaderUnusedKhr)
                                .setIntersectionShader(vk::ShaderUnusedKhr);
                        if (m.flags == vk::ShaderStageFlagBits::eRaygenKHR) {
                            rgenCount++;
                        } else {
                            missCount++;
                        }
                        break;
                    default:
                        throw std::runtime_error("");
                }
                rtShaderGroups.emplace_back(groupInfo);
            }
        }
    }

    auto RayTracing::createPipeline() -> void {
        if (mRTPipeline){
            mDevice.waitIdle();
            mDevice.destroy(mRTPipeline);
        }

        createShaderStage();
        rtCreateInfo.setStages(shaderStages)
                .setGroups(rtShaderGroups)
                .setMaxPipelineRayRecursionDepth(5);

        mRTPipeline = vkCreate("create rt pipeline") = [&]{
            return mDevice.createRayTracingPipelineKHR({}, {}, rtCreateInfo, nullptr, mDyDispatcher).value;
        };
    }

} // yic