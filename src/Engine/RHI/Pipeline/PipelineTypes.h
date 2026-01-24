//
// Created by lenovo on 1/21/2026.
//

#ifndef VKCELSHADINGRENDERER_PIPELINETYPES_H
#define VKCELSHADINGRENDERER_PIPELINETYPES_H

#include "Utils/FileOperation.h"

namespace vot::gfx {
    namespace detail {
        enum class ShaderType {
            eVertex, eTessControl, eTessEvaluation, eGeometry, eFragment,
            eCompute,
            eRayGen, eMiss, eClosestHit, eAnyHit, eIntersection, eCallable,
            eTask, eMesh
        };
        enum class RTGroupKind {
            eRayGen, eMiss, eHit, eCallable
        };
        inline auto createShaderModule(const vk::Device* device, const container::string& shaderPath, const ShaderType& type) -> vk::ShaderModule {
            auto makePath = [&](const container::string& ext) -> container::string {
                return core::path::ShaderSpv + shaderPath + ext + ".spv";
            };
            const auto pt = dsl::Match<ShaderType, container::string>{type}
                // Graphics pipeline
                .case_(ShaderType::eVertex,          [&]{ return makePath(".vert"); })
                .case_(ShaderType::eFragment,        [&]{ return makePath(".frag"); })
                .case_(ShaderType::eGeometry,        [&]{ return makePath(".geom"); })
                .case_(ShaderType::eTessControl,     [&]{ return makePath(".tesc"); })
                .case_(ShaderType::eTessEvaluation,  [&]{ return makePath(".tese"); })
                // Compute pipeline
                .case_(ShaderType::eCompute,         [&]{ return makePath(".comp"); })
                // Ray Tracing pipeline
                .case_(ShaderType::eRayGen,           [&]{ return makePath(".rgen"); })
                .case_(ShaderType::eMiss,             [&]{ return makePath(".rmiss"); })
                .case_(ShaderType::eClosestHit,       [&]{ return makePath(".rchit"); })
                .case_(ShaderType::eAnyHit,           [&]{ return makePath(".rahit"); })
                .case_(ShaderType::eIntersection,     [&]{ return makePath(".rint"); })
                .case_(ShaderType::eCallable,         [&]{ return makePath(".rcall"); })
                // Mesh shading
                .case_(ShaderType::eTask,             [&]{ return makePath(".task"); })
                .case_(ShaderType::eMesh,             [&]{ return makePath(".mesh"); })
                .unreachable_default();

            container::vector<char> v;
            std::ranges::copy(fo::loadFile(pt), std::back_inserter(v));

            return device->createShaderModule(vk::ShaderModuleCreateInfo()
                .setCodeSize(v.size() * sizeof(char))
                .setPCode(reinterpret_cast<const std::uint32_t*>(v.data())));
        }
    }

    namespace api {
        //
        struct PipelineColorBlendAttachmentStateCI {
            const vk::ColorComponentFlags colorFlags = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                                   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            const vk::Bool32 blendEnable = vk::True;
            const vk::BlendFactor srcColorBF = vk::BlendFactor::eSrcAlpha;
            const vk::BlendFactor dstColorBF = vk::BlendFactor::eOneMinusSrcAlpha;
            const vk::BlendOp colorBlendOp = vk::BlendOp::eAdd;
            const vk::BlendFactor srcAlphaBF = vk::BlendFactor::eSrcAlpha;
            const vk::BlendFactor dstAlphaBF = vk::BlendFactor::eOneMinusSrcAlpha;
            const vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd;
        };

        //
        enum class DescriptorLayoutMode {
            eInvalid,
            eLegacyPool,
            eModernIndexing,
        };
        enum class GlobalBindingPolicy {
            eInclude, eExclude,
        };
        struct PipelineDescriptorSetLayoutCI {
            container::map<std::uint32_t, container::vector<vk::DescriptorSetLayoutBinding>> setLayoutBindings;
            container::vector<vk::PushConstantRange> pushConstantRanges;
            // legacy
            container::vector<vk::DescriptorPoolSize> poolSizes;
            std::uint32_t maxSets = 0;
            // model
            DescriptorLayoutMode mode = DescriptorLayoutMode::eInvalid;

            auto& bindDescriptorSetLayoutBinding(const std::uint32_t& setIndex, const std::uint32_t& binding, const vk::DescriptorType& type, const vk::ShaderStageFlags& flags, const std::uint32_t& count = 1) {
                setLayoutBindings[setIndex].emplace_back(binding, type, count, flags);

                if (mode == DescriptorLayoutMode::eLegacyPool) {
                    poolSizes.emplace_back(type, count);
                    if (maxSets < setIndex) maxSets = setIndex;
                }
                return *this;
            }

            auto& setMustConfigure(const DescriptorLayoutMode& dlMode, const GlobalBindingPolicy& global) {
                mode = dlMode;
                if (global == GlobalBindingPolicy::eInclude) {
                    bindDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR);
                    bindDescriptorSetLayoutBinding(0, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
                }
                return *this;
            }

            auto& bindPushConstantRange(const vk::PushConstantRange& range) { pushConstantRanges.emplace_back(range); return *this; }
            auto validate() const { if (mode == DescriptorLayoutMode::eInvalid) throw std::runtime_error("PipelineDescriptorSetLayoutCI: mode must be explicitly set"); }
        };

        //
        struct ComputePipelineCI {
            container::string shaderPath;
            PipelineDescriptorSetLayoutCI pdSetLayoutCI;

            auto& bindComputerShader(const container::string& path) { shaderPath = path; return *this; }
            auto& setPipelineDescriptorSetLayoutCI(const PipelineDescriptorSetLayoutCI& layout) { pdSetLayoutCI = layout; return *this; }
        };

        struct RTShaderGroup {
            vk::RayTracingShaderGroupTypeKHR type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
            std::uint32_t generalShader = vk::ShaderUnusedKHR;
            std::uint32_t closestHit = vk::ShaderUnusedKHR;
            std::uint32_t anyHitShader = vk::ShaderUnusedKHR;
            std::uint32_t intersectionShader = vk::ShaderUnusedKHR;
        };
        struct RayTracingPipelineCI {
            PipelineDescriptorSetLayoutCI pdSetLayoutCI;
            container::unordered_map<detail::ShaderType, container::vector<container::string>> shaderPaths;
            container::unordered_map<detail::RTGroupKind, container::vector<RTShaderGroup>> shaderGroups;

            // Only Hit shaders (ClosestHit / AnyHit / Intersection) are order-sensitive
            // with respect to RTShaderGroup index calculation.
            // Other shader stages are single-role per group and are not affected
            // by registration order.
            auto& bindRayGenShader(const container::string& shader) { shaderPaths[detail::ShaderType::eRayGen].emplace_back(shader); return *this; }
            auto& bindMissShader(const container::string& shader) { shaderPaths[detail::ShaderType::eMiss].emplace_back(shader); return *this; }
            auto& bindClosestHitShader(const container::string& shader) { shaderPaths[detail::ShaderType::eClosestHit].emplace_back(shader); return *this; }
            auto& bindAnyHitShader(const container::string& shader) { shaderPaths[detail::ShaderType::eAnyHit].emplace_back(shader); return *this; }
            auto& bindIntersectionShader(const container::string& shader) { shaderPaths[detail::ShaderType::eIntersection].emplace_back(shader); return *this; }
            auto& bindCallableShader(const container::string& shader) { shaderPaths[detail::ShaderType::eCallable].emplace_back(shader); return *this; }
            auto& bindRayGenGroup(const RTShaderGroup& group) { shaderGroups[detail::RTGroupKind::eRayGen].emplace_back(group); return *this; }
            auto& bindMissGroup(const RTShaderGroup& group) { shaderGroups[detail::RTGroupKind::eMiss].emplace_back(group); return *this; }
            auto& bindHitGroup(const RTShaderGroup& group) { shaderGroups[detail::RTGroupKind::eHit].emplace_back(group); return *this; }
            auto& bindCallableGroup(const RTShaderGroup& group) { shaderGroups[detail::RTGroupKind::eCallable].emplace_back(group); return *this; }
            auto& setPipelineDescriptorSetLayoutCI(const PipelineDescriptorSetLayoutCI& layout) { pdSetLayoutCI = layout; return *this; }
        };


    }
}

#endif //VKCELSHADINGRENDERER_PIPELINETYPES_H