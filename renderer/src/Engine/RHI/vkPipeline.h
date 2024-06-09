//
// Created by lenovo on 6/9/2024.
//

#ifndef VKCELSHADINGRENDERER_VKPIPELINE_H
#define VKCELSHADINGRENDERER_VKPIPELINE_H

#include "Pipeline/GraphicsPipeline.h"
#include "Engine/Core/FileOperator/ShaderHotReloader.h"

namespace yic {

    template<typename state>
    class vkPipeline : public state{
    public:
        template<typename... Args>
        explicit vkPipeline(Args&&...args) : state(std::forward<Args>(args)...){}
        vkPipeline(vk::Device device, vk::PipelineLayout layout, vk::RenderPass renderPass) : state(device, layout, renderPass){}
    };

    void x(){
        vk::Device d;
        vk::PipelineLayout l;
        vk::RenderPass r;
        std::unique_ptr<vkPipeline<Graphics>> p = std::make_unique<vkPipeline<Graphics>>(d, l, r);

    }

//    structgetmputeState : public State<ComputeState>{};
//    struct RayTracingState : public State<RayTracingState>{};
//
//
//    struct UseGeometryShader{};
//    struct UseRayTracing{};
//    struct EnableMultiSampling{};
//
//    template<typename T>
//    struct GeometryShaderTrait{
//        static constexpr int v = 0;
//    };
//    template<>
//    struct GeometryShaderTrait<UseGeometryShader>{
//        static constexpr int v = 1;
//    };
//
//    struct GraphicsStateWithGeometry : UseGeometryShader{};
//
//    template<typename state, int GeometryShaderFeature = GeometryShaderTrait<state>::v, int MultisamplingFeature = 0>
//    class vkPipeline : public state {
//        template<typename T, typename = void>
//        struct initializer{ static void init(){} };
//        template<typename T>
//        struct initializer<T, std::enable_if_t<std::is_same_v<T, Graphics>>>{
//            static void init(){}
//        };
//    public:
//        vkPipeline() : state(){
//            initializer<state>::init();
//        }
//
//    private:
//        template<typename T>
//        typename std::enable_if_t<std::is_same_v<T, UseGeometryShader>, void>
//        setupGeometry(){
//
//        }
//    };

} // yic

#endif //VKCELSHADINGRENDERER_VKPIPELINE_H
