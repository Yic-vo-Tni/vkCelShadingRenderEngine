//
// Created by lenovo on 7/25/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERGROUP_H
#define VKCELSHADINGRENDERER_RENDERGROUP_H

#include "Engine/Core/DispatchSystem/Schedulers.h"

#include "Engine/RHI/vkPipeline.h"
#include "Engine/RHI/Descriptor.h"
#include "Engine/RHI/Command.h"

#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/ECS/Camera/Camera.h"
#include "Engine/ECS/Model/ModelLoader.h"

namespace yic {

    template<typename T>
    concept isGraphics = std::is_same_v<T, Graphics>;
    template<typename T>
    concept isRT = std::is_same_v<T, RayTracing>;

    template<typename PipelineType>
    class RenderGroupBase
            : public PipelineDesSetLayout, public PipelineType, public std::enable_shared_from_this<RenderGroupBase<PipelineType>> {
    public:
        explicit RenderGroupBase(vk::RenderPass rp) requires isGraphics<PipelineType>
                : PipelineDesSetLayout({*mg::SystemHub.val<ev::pVkSetupContext>().device}), PipelineType(rp){
        }

        explicit RenderGroupBase() requires isRT<PipelineType>
                : PipelineDesSetLayout({*mg::SystemHub.val<ev::pVkSetupContext>().device}), RayTracing() {
        }

        ~RenderGroupBase() = default;

        std::shared_ptr<RenderGroupBase<PipelineType>> addDesSetLayout_(const uint32_t &set, const uint32_t &binding, const vk::DescriptorType &descriptorType,
                                                                         const uint32_t &descriptorCount, const vk::ShaderStageFlags &flags){
            PipelineDesSetLayout::addDesSetLayout(set, binding, descriptorType, descriptorCount, flags);
            return this->shared_from_this();
        }
        std::shared_ptr<RenderGroupBase<PipelineType>> addDesSetLayout_(const uint32_t &set, const uint32_t &binding, const vk::DescriptorType &descriptorType,
                                                                        const vk::ShaderStageFlags &flags){
            PipelineDesSetLayout::addDesSetLayout(set, binding, descriptorType, 1, flags);
            return this->shared_from_this();
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> addPushConstantRange_(const vk::ShaderStageFlags &flags, uint32_t offset, uint32_t size) {
            PipelineDesSetLayout::addPushConstantRange(flags, offset, size);

            return this->shared_from_this();
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> addBindingDescription_(const uint32_t &binding, const uint32_t &stride,
                               const vk::VertexInputRate &inputRate) requires isGraphics<PipelineType> {
            Graphics::addBindingDescription(binding, stride, inputRate);
            return this->shared_from_this();
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> addAttributeDescription_(const uint32_t &location, const uint32_t &binding, const vk::Format &format,
                                                                            const uint32_t &offset) {
            Graphics::addAttributeDescription(location, binding, format, offset);
            return this->shared_from_this();
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> addShader_(const std::string& path, vk::ShaderStageFlagBits flags) {
            PipelineType::addShader(path, flags);
            return this->shared_from_this();
        }

        static std::shared_ptr<RenderGroupBase<PipelineType>> configure(const vk::RenderPass& renderPass) requires isGraphics<PipelineType>{
            return std::make_shared<RenderGroupBase<PipelineType>>(renderPass);
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> bindDescriptor(std::shared_ptr<Descriptor>& descriptor){
            descriptor = Descriptor::configure(*this);
            return this->shared_from_this();
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> build() requires isGraphics<PipelineType> {
            Graphics::createInfo.setLayout(PipelineDesSetLayout::getPipelineLayout());
            Graphics::create();
            return this->shared_from_this();
        }

        static std::shared_ptr<RenderGroupBase<PipelineType>> configure() requires isRT<PipelineType> {
            return std::make_shared<RenderGroupBase<RayTracing>>();
        }

        std::shared_ptr<RenderGroupBase<PipelineType>> build() requires isRT<PipelineType>{
            RayTracing::rtCreateInfo.setLayout(PipelineDesSetLayout::getPipelineLayout());
            RayTracing::create();
            return this->shared_from_this();
        }
    };

    using RenderGroupGraphics = RenderGroupBase<Graphics>;
    using RenderGroupRayTracing = RenderGroupBase<RayTracing>;

    using RenderGroupGraphics_sptr = std::shared_ptr<RenderGroupGraphics>;
    using RenderGroupRayTracing_sptr = std::shared_ptr<RenderGroupRayTracing>;

//    class RenderGroupCombine{
//    public:
//        auto configureGraphicsPipeline(const vk::RenderPass& rp = {}) -> RenderGroupGraphics_sptr {
//            if (graphicsSptr == nullptr){
//                if (!rp){ throw std::runtime_error("First configure graphics pipeline must include a valid render pass");}
//                graphicsSptr = RenderGroupGraphics ::configure(rp);
//            }
//            return graphicsSptr;
//        }
//
//        auto configureRayTracingPipeline() -> RenderGroupRayTracing_sptr {
//            if (rayTracingSptr == nullptr){
//                rayTracingSptr = RenderGroupRayTracing ::configure();
//            }
//            return rayTracingSptr;
//        }
//
//        auto graphics() {
//            return graphicsSptr;
//        }
//        auto rayTracing() {
//            return rayTracingSptr;
//        }
//
//        auto clear() -> void{
//            graphicsSptr.reset();
//            rayTracingSptr.reset();
//        }
//    private:
//        RenderGroupGraphics_sptr graphicsSptr;
//        RenderGroupRayTracing_sptr rayTracingSptr;
//    };





} // yic

#endif //VKCELSHADINGRENDERER_RENDERGROUP_H
