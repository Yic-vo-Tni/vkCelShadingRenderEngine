//
// Created by lenovo on 7/25/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERGROUP_H
#define VKCELSHADINGRENDERER_RENDERGROUP_H

#include "Engine/RHI/vkPipeline.h"
#include "Engine/RHI/Descriptor.h"

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/ECS/Model/LoadModel.h"

namespace yic {

    struct Render{
        Render(const vk::CommandBuffer &cmd, const vk::Pipeline &pipe,
               const vk::PipelineLayout &pipeLayout,
               const std::vector<sc::Model>& models) : cmd(cmd),
                                                       mPipeline(pipe),
                                                       mPipelineLayout(pipeLayout),
                                                       mModels(models){
        }
        Render& bindPipeline(){
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
            return *this;
        }
        template<typename T>
        Render& pushConstants(const vk::ShaderStageFlags& flags, uint32_t offset, uint32_t size, const T value){
            cmd.pushConstants(mPipelineLayout, flags, offset, size, &value);
            return *this;
        }

        Render &drawModel() {
            for (auto &model: mModels) {
                for (const auto &mesh: model.meshes) {
                    std::vector<vk::DeviceSize> offsets{0};
                    std::vector<vk::Buffer> buffers{mesh.vertBuf->buffer};
                    cmd.bindVertexBuffers(0, buffers, offsets);
                    cmd.bindIndexBuffer(mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout,
                                           0, model.descriptor->getDescriptorSets()[mesh.texIndex],
                                           nullptr);
                    cmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);
                }
//                oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<uint32_t>(0, model.meshes.size()),
//                                          [&](const oneapi::tbb::blocked_range<uint32_t> &range) {
//                                              for (auto i = range.begin(); i != range.end(); i++) {
//                                                  const auto &mesh = model.meshes[i];
//                                                  std::vector<vk::DeviceSize> offsets{0};
//                                                  std::vector<vk::Buffer> buffers{mesh.vertBuf->buffer};
//                                                  cmd.bindVertexBuffers(0, buffers, offsets);
//                                                  cmd.bindIndexBuffer(mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
//                                                  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
//                                                                         mPipelineLayout,
//                                                                         0,
//                                                                         model.descriptor->getDescriptorSets()[mesh.texIndex],
//                                                                         nullptr);
//                                                  cmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);
//                                              }
//                                          });
            }
            return *this;
        }
    private:
        vk::CommandBuffer cmd;
        vk::Pipeline mPipeline;
        vk::PipelineLayout mPipelineLayout;
        std::vector<sc::Model> mModels;
    };

    class RenderGroup : public PipelineDesSetLayout, public vkPipeline<Graphics>, public std::enable_shared_from_this<RenderGroup>{
    public:
        explicit RenderGroup(const vk::RenderPass& renderPass);
        ~RenderGroup() = default;

        std::shared_ptr<RenderGroup> addDesSetLayout_1(const uint32_t &set, const uint32_t &binding, const vk::DescriptorType &descriptorType,
                const uint32_t &descriptorCount, const vk::ShaderStageFlags &flags);
        std::shared_ptr<RenderGroup> addPushConstantRange_2(const vk::ShaderStageFlags &flags, uint32_t offset, uint32_t size) ;
        std::shared_ptr<RenderGroup> addBindingDescription_3(const uint32_t &binding, const uint32_t &stride, const vk::VertexInputRate &inputRate = vk::VertexInputRate::eVertex) ;
        std::shared_ptr<RenderGroup> addAttributeDescription_4(const uint32_t &location, const uint32_t &binding, const vk::Format &format, const uint32_t &offset) ;
        std::shared_ptr<RenderGroup> addShader_5(const std::string& path, vk::ShaderStageFlagBits flags) ;
        std::shared_ptr<RenderGroup> build();

        std::shared_ptr<RenderGroup> addModel(const std::string& path){
            mModels.emplace_back(sc::ModelLoader::Load(path, *this));
            return shared_from_this();
        }

        static std::shared_ptr<RenderGroup> configure(const vk::RenderPass& renderPass){
            return std::make_shared<RenderGroup>(renderPass);
        }

        Render render(const vk::CommandBuffer& cmd){
            return {cmd, mPipeline, getPipelineLayout(), mModels};
        }


    private:
        std::vector<sc::Model> mModels;
    };






} // yic

#endif //VKCELSHADINGRENDERER_RENDERGROUP_H
