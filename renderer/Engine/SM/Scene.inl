//
// namespace sm {
//
//     template<vot::VertexType type>
//     auto SceneSystem::syncBLAS(const vot::VertexDataComponent<type> &vdc, vot::RenderComponent &rc, vot::RayTracingComponent &rtc,
//                    bool update = false) -> void {
//                 auto vertAddr = rc.vertexBuffer[yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur()]->bufferAddr();
//         auto indexAddr = rc.indexBuffer->bufferAddr();
//
//         uint32_t maxVert{}, numTri{};
//         if (vc.isMMD){
//             maxVert = (uint32_t ) vc.pmx->GetVertexCount();
//             numTri = (uint32_t ) vc.pmx->GetIndexCount() / 3;
//         } else {
//             maxVert = (uint32_t) vc.vertices_pmr[yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur()].size();
//             numTri = (uint32_t) vc.indices_pmr.size() / 3;
//         }
//
//         auto asGeomTriData = vk::AccelerationStructureGeometryTrianglesDataKHR()
//                 .setVertexFormat(vk::Format::eR32G32B32Sfloat)
//                 .setVertexData(vertAddr)
//                 .setMaxVertex(maxVert)
//                 .setIndexData(indexAddr);
//         if (vc.isMMD){
//             asGeomTriData.setVertexStride(sizeof(vot::VertexT<vot::eMMD>))
//             .setIndexType(rc.indexType);
//         } else {
//             asGeomTriData.setVertexStride(sizeof (vot::VertexT<vot::eAssimp>))
//             .setIndexType(vk::IndexType::eUint32);
//         }
//
//         auto asGeom = vk::AccelerationStructureGeometryKHR()
//                 .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
//                 .setGeometryType(vk::GeometryTypeKHR::eTriangles)
//                 .setGeometry(vk::AccelerationStructureGeometryDataKHR()
//                 .setTriangles(asGeomTriData));
//
//         auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
//                 .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
//                 .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate)
//                 .setGeometries(asGeom);
//
//         auto asBuildSizeInfo = ct.device->getAccelerationStructureBuildSizesKHR(
//                 vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, numTri, *ct.dynamicDispatcher);
//
//         if (!update){
//             rtc.blas = yic::allocator->allocAccel(asBuildSizeInfo.accelerationStructureSize,
//                                                   vk::AccelerationStructureTypeKHR::eBottomLevel);
//
//             rtc.scratchBuffer = yic::allocator->allocDedicatedBufferStaging(asBuildSizeInfo.buildScratchSize,
//                                                                           vk::BufferUsageFlagBits::eShaderDeviceAddress |
//                                                                           vk::BufferUsageFlagBits::eStorageBuffer,
//                                                                           " blas scratch");
//
//             asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
//                     .setDstAccelerationStructure(rtc.blas->accel)
//                     .setScratchData(rtc.scratchBuffer->bufferAddr());
//         } else {
//             asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eUpdate)
//                 .setSrcAccelerationStructure(rtc.blas->accel)
//                 .setDstAccelerationStructure(rtc.blas->accel)
//                 .setScratchData(rtc.scratchBuffer->bufferAddr());
//         }
//
//         auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
//                 .setFirstVertex(0)
//                 .setPrimitiveOffset(0)
//                 .setPrimitiveCount(numTri)
//                 .setTransformOffset(0);
//
//         yic::command->drawOneTimeSubmit([&](vot::CommandBuffer &cmd) {
//             cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, *ct.dynamicDispatcher);
//             vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR,
//                                       vk::AccessFlagBits::eAccelerationStructureReadKHR};
//             cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
//                                 vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
//                                 {}, barrier, {}, {});
//         });
//     };
//
// }