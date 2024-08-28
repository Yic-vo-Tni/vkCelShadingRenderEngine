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

    RayTracing::RayTracing(): mDevice(EventBus::Get::vkSetupContext().device_ref()),
            mDyDispatcher(EventBus::Get::vkSetupContext().dynamicDispatcher_ref()) {
        vk::PhysicalDeviceProperties2 properties2{};
        properties2.pNext = &rtProperties;
        EventBus::Get::vkSetupContext().physicalDevice_ref().getProperties2(&properties2);
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
        auto handleCount = rtShaderGroups.size();
        auto handleSize = rtProperties.shaderGroupHandleSize;
        auto handleSizeAligned = align_up(handleSize, rtProperties.shaderGroupHandleAlignment);

        regionRgen.stride = align_up(handleSizeAligned, rtProperties.shaderGroupBaseAlignment);
        regionRgen.size = regionRgen.stride;

        regionMiss.stride = handleSizeAligned;
        regionMiss.size = align_up(missCount * handleSizeAligned, rtProperties.shaderGroupBaseAlignment);

        regionHit.stride = handleSizeAligned;
        regionHit.size = align_up(hitCount * handleSizeAligned, rtProperties.shaderGroupBaseAlignment);
//        regionRgen.stride = handleSize;
//        regionRgen.size = handleSize;
//
//        regionMiss.stride = handleSize;
//        regionMiss.size = missCount * handleSize;
//
//        regionHit.size = handleSize;
//        regionHit.size = hitCount * handleSize;

        auto dataSize = handleCount * handleSize;

        std::vector<uint8_t> handles = mDevice.getRayTracingShaderGroupHandlesKHR<uint8_t>(mRTPipeline, 0, handleCount, dataSize, mDyDispatcher);

        auto sbtSize = regionRgen.size + regionMiss.size + regionHit.size + regionCall.size;

        sbtBuf = Allocator::allocBuf(sbtSize, vk::BufferUsageFlagBits::eTransferSrc |
                                                   vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                   vk::BufferUsageFlagBits::eShaderBindingTableKHR,
                                          Allocator::MemoryUsage::eCpuToGpu, IdGenerator::uniqueId());

        auto sbtAddr = mDevice.getBufferAddress(vk::BufferDeviceAddressInfo{sbtBuf->buffer});
        regionRgen.deviceAddress = sbtAddr;
        regionMiss.deviceAddress = sbtAddr + regionRgen.size;
        regionHit.deviceAddress = sbtAddr + regionRgen.size + regionMiss.size;


        auto getHandle = [&](int i) { return handles.data() + i * handleSize; };

        std::vector<uint8_t> tempBuf(sbtSize);
        auto* pData = tempBuf.data();
        auto index{0};

        memcpy(pData, getHandle(index++), handleSize);
        pData += regionRgen.size;

        for(auto i = 0; i < missCount; i++){
            memcpy(pData, getHandle(index++), handleSize);
            pData += regionMiss.stride;
        }
        for(auto i = 0; i < hitCount; i++){
            memcpy(pData, getHandle(index++), handleSize);
            pData += regionHit.stride;
        }

        sbtBuf->updateBuf(tempBuf);
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

                if (shaderModules[path].shaderModule)
                    mDevice.destroy(shaderModules[path].shaderModule);

                shaderModules[path].shaderModule = mDevice.createShaderModule(vk::ShaderModuleCreateInfo().setCodeSize(sizeof (char) * v.size())
                                                                                      .setPCode(reinterpret_cast<const uint32_t*>(v.data())));
                shaderModules[path].flags = flags;
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

        std::vector<std::pair<std::string, ShaderModule>> sortedModules;

        for (auto& [p, m] : shaderModules) {
            if (m.flags == vk::ShaderStageFlagBits::eRaygenKHR) {
                sortedModules.emplace_back(p, m);
            }
        }

        for (auto& [p, m] : shaderModules) {
            if (m.flags == vk::ShaderStageFlagBits::eMissKHR) {
                sortedModules.emplace_back(p, m);
            }
        }

        for (auto& [p, m] : shaderModules) {
            if (m.flags == vk::ShaderStageFlagBits::eClosestHitKHR) {
                sortedModules.emplace_back(p, m);
            }
        }

        for(auto& [p, m] : sortedModules){
            vk::PipelineShaderStageCreateInfo shaderStage{{}, m.flags, m.shaderModule, "main"};
            shaderStages.push_back(shaderStage);

            if (shaderTypeMap.count(m.flags) > 0){
                vk::RayTracingShaderGroupCreateInfoKHR groupInfo{};
                groupInfo.setType(shaderTypeMap[m.flags]);

                switch (m.flags) {
                    case vk::ShaderStageFlagBits::eClosestHitKHR:
                        groupInfo.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
                        groupInfo.setClosestHitShader(shaderStages.size() - 1)
                                .setGeneralShader(vk::ShaderUnusedKhr)
                                .setAnyHitShader(vk::ShaderUnusedKhr)
                                .setIntersectionShader(vk::ShaderUnusedKhr);
                        hitCount ++;
                        break;
                    case vk::ShaderStageFlagBits::eRaygenKHR:
                    case vk::ShaderStageFlagBits::eMissKHR:
                        groupInfo.setGeneralShader(shaderStages.size() - 1)
                                .setClosestHitShader(vk::ShaderUnusedKhr)
                                .setAnyHitShader(vk::ShaderUnusedKhr)
                                .setIntersectionShader(vk::ShaderUnusedKhr);
                        if (m.flags == vk::ShaderStageFlagBits::eRaygenKHR){
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

        auto& x= rtCreateInfo;
        createShaderStage();
        rtCreateInfo.setStages(shaderStages)
                .setGroups(rtShaderGroups)
                .setMaxPipelineRayRecursionDepth(2);

        mRTPipeline = vkCreate("create rt pipeline") = [&]{
            return mDevice.createRayTracingPipelineKHR({}, {}, rtCreateInfo, nullptr, mDyDispatcher).value;
        };
    }

} // yic