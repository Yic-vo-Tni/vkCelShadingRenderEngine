///
// Created by lenovo on 8/11/2024.
//

#include "RTBuilder.h"

namespace yic {

    template<class integral>
    constexpr integral align_up(integral x, size_t a) noexcept{
        return integral((x + (integral(a) - 1)) & ~integral(a - 1));
    }

    RTBuilder::RTBuilder() {
        EventBus::valAuto(mDevice, mPhysicalDevice, mDyDispatcher);

        cBLAS();
        cTLAS();
//        cRtPipe();
//        cSbt();

        mExtent.setWidth(2560).setHeight(1440);
        Allocator::allocImgAuto(offRtImg, vk::ImageUsageFlagBits::eStorage, mExtent);
    }
    RTBuilder::~RTBuilder() {
        mDevice.destroy(desSetLayout);
        mDevice.destroy(pipeLayout);
        mDevice.destroy(rtPipeline);
        mDevice.destroy(descriptorPool);
    }

    auto RTBuilder::cBLAS() -> void {
//        struct Vertex{
//            float pos[3];
//        };
//
//
//        auto vertices = {
//                Vertex{ 1.f,  1.f, 0.f},
//                      {-1.f,  1.f, 0.f},
//                      { 0.f, -1.f, 0.f},
//        };
//        auto indices = vot::uint32L {
//                0, 1, 2
//        };
//
//        auto tfMatrix = vk::TransformMatrixKHR().matrix = {
//                {{
//                         {1.0f, 0.0f, 0.0f, 0.0f},
//                         {0.0f, 1.0f, 0.0f, 0.0f},
//                         {0.0f, 0.0f, 1.0f, 0.0f}
//                 }}
//        };
//
//        Allocator::allocBufAuto(vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
//                              vertBuf, vertices,
//                              indexBuf, indices,
//                              tfMatrixBuf, sizeof(tfMatrix), &tfMatrix);
//
//        INIT_MULTI(vk::DeviceOrHostAddressConstKHR,
//                   vertBufAddr{Allocator::getBufAddr(vertBuf)},
//                   indexBufAddr{Allocator::getBufAddr(indexBuf)},
//                   transformBufAddr{Allocator::getBufAddr(tfMatrixBuf)});
//
//        auto asGeom = vk::AccelerationStructureGeometryKHR()
//                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
//                .setGeometryType(vk::GeometryTypeKHR::eTriangles)
//                .setGeometry(
//                        vk::AccelerationStructureGeometryDataKHR().setTriangles(
//                                vk::AccelerationStructureGeometryTrianglesDataKHR()
//                                        .setVertexFormat(vk::Format::eR32G32B32Sfloat)
//                                        .setVertexData(vertBufAddr)
//                                        .setMaxVertex(3)
//                                        .setVertexStride(sizeof(Vertex))
//                                        .setIndexType(vk::IndexType::eUint32)
//                                        .setIndexData(indexBufAddr)
//                                        .setTransformData(transformBufAddr)
//                        )
//                );
//
//        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
//                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
//                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
//                .setGeometries(asGeom);
//
//        const uint32_t numTriangles = 1;
//
//        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, numTriangles, mDyDispatcher);
//
//        auto asCreateInfo = vk::AccelerationStructureCreateInfoKHR()
//                .setSize(asBuildSizeInfo.accelerationStructureSize)
//                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
//        blas = Allocator::allocAccel(asCreateInfo);
//
//        auto scratchBuf = Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
//        vk::BufferDeviceAddressInfo bufferInfo{scratchBuf->buffer};
//        vk::DeviceAddress  scratchAddress = mDevice.getBufferAddress(bufferInfo);
//
//        asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
//                    .setDstAccelerationStructure(blas->accel)
//                    .setGeometries(asGeom)
//                    .setScratchData(scratchAddress);
//
//        auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
//                .setPrimitiveCount(numTriangles)
//                .setPrimitiveOffset(0)
//                .setFirstVertex(0)
//                .setTransformOffset(0);
//
//
//        CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
//           cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, mDyDispatcher);
//        });

        auto& m = (mModel = sc::ModelLoader::Load(R"(E:\Material\model\Nilou\Nilou.pmx)")).mesh;
        INIT_MULTI(vk::DeviceOrHostAddressConstKHR, vertBufAddr{Allocator::getBufAddr(m.vertBuf)},
                                                    indexBufAddr{Allocator::getBufAddr(m.indexBuf)});

        auto maxVert = (uint32_t )m.vertices.size();
        auto numTri = (uint32_t )m.indices.size() / 3;

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometryType(vk::GeometryTypeKHR::eTriangles)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                .setTriangles(vk::AccelerationStructureGeometryTrianglesDataKHR()
                .setVertexFormat(vk::Format::eR32G32B32Sfloat)
                .setVertexData(vertBufAddr)
                .setVertexStride(sizeof(sc::Vertex))
                .setMaxVertex(maxVert)
                .setIndexType(vk::IndexType::eUint32)
                .setIndexData(indexBufAddr)));
        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, numTri, mDyDispatcher);


        blas = Allocator::allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eBottomLevel);
//        auto asCreateInfo = vk::AccelerationStructureCreateInfoKHR()
//                .setSize(asBuildSizeInfo.accelerationStructureSize)
//                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
//        blas = Allocator::allocAccel(asCreateInfo);

        auto scratchBuf = Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
        auto scratchBufAddr = vk::DeviceOrHostAddressKHR{Allocator::getBufAddr(scratchBuf)};

        asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                .setDstAccelerationStructure(blas->accel)
                .setScratchData(scratchBufAddr);

        auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setFirstVertex(0)
                .setPrimitiveOffset(0)
                .setPrimitiveCount(numTri)
                .setTransformOffset(0);

        CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
            cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, mDyDispatcher);
            vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                      vk::AccessFlagBits::eAccelerationStructureReadKHR};
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                {}, barrier, {}, {});
        });
    }

    auto RTBuilder::cTLAS() -> void {
//        auto tfMatrix = Allocator::glmMatToVkTransformMatrix();
//
//        auto inst = vk::AccelerationStructureInstanceKHR()
//                .setTransform(tfMatrix)
//                .setInstanceCustomIndex(0)
//                .setMask(0xFF)
//                .setInstanceShaderBindingTableRecordOffset(0)
//                .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
//                .setAccelerationStructureReference(Allocator::getAccelDevAddr(blas));
//
//        auto instBuf = Allocator::allocBufStaging(sizeof(vk::AccelerationStructureInstanceKHR), &inst,
//                                                  vk::BufferUsageFlagBits::eShaderDeviceAddress |
//                                                  vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
//                                                  Allocator::MemoryUsage::eCpuToGpu);
//
//        vk::DeviceOrHostAddressConstKHR instDevAddr{Allocator::getBufAddr(instBuf)};
//
//        auto asGeom = vk::AccelerationStructureGeometryKHR()
//                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
//                .setGeometryType(vk::GeometryTypeKHR::eInstances)
//                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
//                                     .setInstances(
//                                             vk::AccelerationStructureGeometryInstancesDataKHR()
//                                                     .setArrayOfPointers(vk::False)
//                                                     .setData(instDevAddr))
//                );
//
//        auto asGeomBuildInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
//                .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
//                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
//                .setGeometries(asGeom);
//
//        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asGeomBuildInfo, 1, mDyDispatcher);
//
//        auto asCreateInfo = vk::AccelerationStructureCreateInfoKHR()
//                .setSize(asBuildSizeInfo.accelerationStructureSize)
//                .setType(vk::AccelerationStructureTypeKHR::eTopLevel);
//
//        tlas = Allocator::allocAccel(asCreateInfo);
//
//        auto scratchBuf = Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
//        auto scratchAddr = Allocator::getBufAddr(scratchBuf);
//
//        asGeomBuildInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
//                .setDstAccelerationStructure(tlas->accel)
//                .setScratchData(scratchAddr);
//
//        auto asBuildRangInfo = vk::AccelerationStructureBuildRangeInfoKHR()
//                .setPrimitiveCount(1)
//                .setPrimitiveOffset(0)
//                .setFirstVertex(0)
//                .setTransformOffset(0);
//
//        CommandBufferCoordinator::buildAccelerationStructuresKHR(asGeomBuildInfo, &asBuildRangInfo);

        auto tfMat = Allocator::glmMatToVkTransformMatrix();

        auto inst = vk::AccelerationStructureInstanceKHR()
                .setTransform(tfMat)
                .setInstanceCustomIndex(0)
                .setMask(0xFF)
                .setInstanceShaderBindingTableRecordOffset(0)
                .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                .setAccelerationStructureReference(Allocator::getAccelDevAddr(blas));

        auto instBuf = Allocator::allocBufStaging(sizeof(vk::AccelerationStructureInstanceKHR), &inst,
                                                  vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                  vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setGeometryType(vk::GeometryTypeKHR::eInstances)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                .setInstances(vk::AccelerationStructureGeometryInstancesDataKHR()
                .setArrayOfPointers(vk::False)
                .setData(Allocator::getBufAddr(instBuf))));
        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, 1, mDyDispatcher);

        tlas = Allocator::allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eTopLevel);

        auto scratchBuf = Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize,
                                                     vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);

        asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                .setDstAccelerationStructure(tlas->accel)
                .setScratchData(vk::DeviceOrHostAddressKHR{Allocator::getBufAddr(scratchBuf)});

        auto asBuildRangInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setPrimitiveCount(1)
                .setTransformOffset(0)
                .setPrimitiveOffset(0)
                .setFirstVertex(0);

        CommandBufferCoordinator::buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangInfo);

        vkWarn(sizeof(sc::Vertex));
    }

//    auto RTBuilder::cSbt() -> void {
//        vk::PhysicalDeviceProperties2 properties2{{}, &rtProperties};
//        mPhysicalDevice.getProperties2(&properties2);
//        const uint32_t hs = rtProperties.shaderGroupHandleSize;
//        const uint32_t hsAligned = align_up(rtProperties.shaderGroupHandleSize, rtProperties.shaderGroupHandleAlignment);
//        const uint32_t numGroup = shaderGroups.size();
//        const uint32_t sbtSize = numGroup * hsAligned;
//
//        auto handles = mDevice.getRayTracingShaderGroupHandlesKHR<uint8_t >(rtPipeline, 0, numGroup, sbtSize, mDyDispatcher);
//        auto bufUsage = vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
//
//        Allocator::allocBufAuto(rgenSbt, hs, handles.data(), bufUsage,
//                                rmissSbt, handles.data() + hsAligned,
//                                rchitSbt, handles.data() + hsAligned * 2);
//    }
//
//    auto RTBuilder::cRtPipe() -> void {
//        auto binding = [&](uint32_t binding, vk::DescriptorType type, uint32_t desCount, vk::ShaderStageFlagBits flagBits){
//            return vk::DescriptorSetLayoutBinding()
//                    .setBinding(binding)
//                    .setDescriptorType(type)
//                    .setDescriptorCount(desCount)
//                    .setStageFlags(flagBits);
//        };
//        using b = vk::DescriptorSetLayoutBinding;
//        auto bindings = {
//            binding(0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR),
//            binding(1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR),
//            binding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR),
//        };
//
//        auto desSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
//                .setBindings(bindings);
//
//        desSetLayout = mDevice.createDescriptorSetLayout(desSetLayoutCreateInfo);
//
//        auto pipeLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
//                .setSetLayouts(desSetLayout);
//        pipeLayout = mDevice.createPipelineLayout(pipeLayoutCreateInfo);
//
//        vot::vector<vk::UniqueShaderModule> shaderModules;
//        auto shader = [&](const std::string &path, vk::ShaderStageFlagBits flags,
//                          vk::RayTracingShaderGroupTypeKHR typeKhr, shaderGroupType type = eGeneral) {
//            std::vector<char> v;
//            std::ranges::copy(fo::loadFile(path), std::back_inserter(v));
//            auto sm = mDevice.createShaderModuleUnique(vk::ShaderModuleCreateInfo()
//                                                         .setCodeSize(sizeof(char) * v.size())
//                                                         .setPCode(reinterpret_cast<const uint32_t *>(v.data())));
//            shaderStages.emplace_back(vk::PipelineShaderStageCreateInfo()
//                                              .setModule(sm.get())
//                                              .setPName("main")
//                                              .setStage(flags));
//            shaderGroups.emplace_back(vk::RayTracingShaderGroupCreateInfoKHR()
//                                              .setType(typeKhr)
//                                              .setGeneralShader(type == eGeneral ? shaderStages.size() - 1 : vk::ShaderUnusedKhr)
//                                              .setClosestHitShader(type == eClosestHit ? shaderStages.size() - 1: vk::ShaderUnusedKhr)
//                                              .setAnyHitShader(type == eAnyHit ? shaderStages.size() - 1 : vk::ShaderUnusedKhr)
//                                              .setIntersectionShader(type == eIntersection ? shaderStages.size() - 1: vk::ShaderUnusedKhr));
//            shaderModules.emplace_back(std::move(sm));
//        };
//
//        shader(fo::path::spv("b_rt_gen.rgen"), vk::ShaderStageFlagBits::eRaygenKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral);
//        shader(fo::path::spv("b_rt_miss.rmiss"), vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral);
//        shader(fo::path::spv("b_rt_hit.rchit"), vk::ShaderStageFlagBits::eClosestHitKHR, vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup, eClosestHit);
//
//        rtPipeline = vkCreate("rt pipeline", spdlog::level::warn) = [&]{
//            return mDevice.createRayTracingPipelineKHR(nullptr, nullptr, vk::RayTracingPipelineCreateInfoKHR()
//                    .setStages(shaderStages)
//                    .setGroups(shaderGroups)
//                    .setMaxPipelineRayRecursionDepth(1)
//                    .setLayout(pipeLayout), nullptr, mDyDispatcher).value;
//        };
//    }
//
//    auto RTBuilder::cDesSets(const vkBuf_sptr& bufSptr) -> void {
//        auto poolSize = {vk::DescriptorPoolSize
//                         {vk::DescriptorType::eAccelerationStructureKHR, 1},
//                         {vk::DescriptorType::eStorageImage, 1},
//                         {vk::DescriptorType::eUniformBuffer, 1},
//        };
//
//        descriptorPool = mDevice.createDescriptorPool(vk::DescriptorPoolCreateInfo()
//                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
//                .setMaxSets(1)
//                .setPoolSizes(poolSize));
//        descriptorSets = mDevice.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
//                .setSetLayouts(desSetLayout)
//                .setDescriptorPool(descriptorPool)
//                .setDescriptorSetCount(1));
//
//        auto asSet = vk::WriteDescriptorSetAccelerationStructureKHR()
//                .setAccelerationStructures(tlas->accel);
//        auto storageImgSet = vk::DescriptorImageInfo()
//                .setImageView(offRtImg->imageViews.front())
//                .setImageLayout(vk::ImageLayout::eGeneral);
//        auto uniformBuf = vk::DescriptorBufferInfo()
//                .setBuffer(bufSptr->buffer)
//                .setRange(vk::WholeSize)
//                .setOffset(0);
//        auto& desSet = descriptorSets.front();
//
//        auto writeSets = {
//                vk::WriteDescriptorSet{desSet, 0, {}, 1, vk::DescriptorType::eAccelerationStructureKHR, {}, {}, {}, &asSet},
//                {desSet, 1, {}, vk::DescriptorType::eStorageImage, storageImgSet, {}, {}, {}},
//                {desSet, 2, {}, vk::DescriptorType::eUniformBuffer, {}, uniformBuf, {}, {}},
//        };
//
//        mDevice.updateDescriptorSets(writeSets, {});
//    }
//
//    auto RTBuilder::draw(const vk::CommandBuffer &cmd) -> void {
//
//        const auto hsAligned = align_up(rtProperties.shaderGroupHandleSize, rtProperties.shaderGroupHandleAlignment);
//
//        auto rgenRegion = vk::StridedDeviceAddressRegionKHR()
//                .setDeviceAddress(Allocator::getBufAddr(rgenSbt))
//                .setSize(hsAligned)
//                .setStride(hsAligned);
//        auto missRegion = vk::StridedDeviceAddressRegionKHR()
//                .setDeviceAddress(Allocator::getBufAddr(rmissSbt))
//                .setStride(hsAligned)
//                .setSize(hsAligned);
//        auto hitRegion = vk::StridedDeviceAddressRegionKHR()
//                .setDeviceAddress(Allocator::getBufAddr(rchitSbt))
//                .setStride(hsAligned)
//                .setSize(hsAligned);
//        auto callRegion = vk::StridedDeviceAddressRegionKHR();
//
//        cmd.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline);
//        cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, pipeLayout, 0, descriptorSets.front(), {});
//
//        cmd.traceRaysKHR(rgenRegion, missRegion, hitRegion, callRegion, mExtent.width, mExtent.height, 1, mDyDispatcher);
//    }

    auto RTBuilder::cRTPipeAndSBT(const vkBuf_sptr& bufSptr) -> void {
        rayTracingSptr = RenderGroupRayTracing ::configure()
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addDesSetLayout_(0, 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR)
                ->addDesSetLayout_(0, 2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR)
                ->addDesSetLayout_(0, 3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addDesSetLayout_(0, 4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addShader_("b_rt_gen.rgen", vk::ShaderStageFlagBits::eRaygenKHR)
                ->addShader_("b_rt_miss.rmiss", vk::ShaderStageFlagBits::eMissKHR)
                ->addShader_("b_rt_shadow.rmiss", vk::ShaderStageFlagBits::eMissKHR)
                ->addShader_("b_rt_hit.rchit", vk::ShaderStageFlagBits::eClosestHitKHR)
                ->build()
                ;

        descriptor = Descriptor::configure(*rayTracingSptr)
                ->updateDesSetAuto(tlas, ImgInfo{{}, offRtImg, vk::ImageLayout::eGeneral}, bufSptr, mModel.mesh.vertBuf, mModel.mesh.indexBuf);
    }

    auto RTBuilder::drawNew(const vk::CommandBuffer &cmd) -> void {
        cmd.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rayTracingSptr->acquire());
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, rayTracingSptr->getPipelineLayout(), 0, descriptor->getDescriptorSets().front(), {});

        cmd.traceRaysKHR(rayTracingSptr->getRegionRgen(), rayTracingSptr->getRegionMiss(),
                         rayTracingSptr->getRegionHit(), rayTracingSptr->getRegionCall(),
                         mExtent.width, mExtent.height, 1, mDyDispatcher);
    }


} // yic