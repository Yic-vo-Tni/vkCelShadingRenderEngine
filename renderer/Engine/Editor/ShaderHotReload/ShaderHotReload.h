//
// Created by lenovo on 6/10/2025.
//

#ifndef VKCELSHADINGRENDERER_SHADERHOTRELOAD_H
#define VKCELSHADINGRENDERER_SHADERHOTRELOAD_H

#include "ShaderEditor.h"
#include "RHI/Pipeline/GraphicsPipeline.h"
#include "RHI/Pipeline/RayTracingPipeline.h"

namespace ui {

    class ShaderHotReload {
        struct PipeInfo{
            rhi::GraphicsPipeline* gp{};
            rhi::RayTracingPipeline* rp{};
            vk::ShaderStageFlagBits flags{};
        };
    public:
        MAKE_SINGLETON(ShaderHotReload);
        ShaderHotReload() {
            compile();
            shaderEditor = std::make_unique<ShaderEditor>();
        };
        ~ShaderHotReload() = default;

        auto rego(const vot::string& pt, const PipeInfo& info) -> void{
            buildOrders.emplace_back(pt);
            buildTasks[pt] = info;
        }

        auto update(const vot::string& pt) -> void{
            ptsUpdate.emplace_back(pt);
        }


        auto frame(){
            for(auto& p : ptsUpdate){
                if (buildTasks.find(p) != buildTasks.end()){
                    auto& info = buildTasks[p];

                    switch (info.flags) {
                        case vk::ShaderStageFlagBits::eVertex:
                        case vk::ShaderStageFlagBits::eGeometry:
                            info.gp->buildPreRasterizationShadersLibrary();
                            info.gp->build();
                            break;
                        case vk::ShaderStageFlagBits::eFragment:
                            info.gp->buildFragmentShaderLibrary();
                            info.gp->build();
                            break;
                        case vk::ShaderStageFlagBits::eRaygenKHR:
                        case vk::ShaderStageFlagBits::eClosestHitKHR:
                        case vk::ShaderStageFlagBits::eMissKHR:
                        case vk::ShaderStageFlagBits::eAnyHitKHR: {
                            info.rp->clear();
                            auto builds = info.rp->getRebuilds();
                            info.rp->getRebuilds().clear();
                            for (const auto &[path, flags, type, role]: builds) {
                                info.rp->addShader(path, flags, type, role);
                            }
                            info.rp->build();
                        }
                        default:
                            break;
                    }
                }
                ptsUpdate.clear();
            }
        }

        auto tempEditor(const vot::string& pt) -> void{
            shaderEditor->build(pt);
        }

        auto compile() -> void{
            std::string shaderPath = shader_path "/..";

            auto cmake_cmd = "cmake -S " + shaderPath + " -B " + shaderPath + "/build";
            auto build_cmd = "cmake --build " + shaderPath + "/build";

            boost::process::system(cmake_cmd, boost::process::std_out > stdout, boost::process::std_err > stderr);
            boost::process::system(build_cmd, boost::process::std_out > stdout, boost::process::std_err > stderr);
        }

        const auto& getBuildTasks() { return buildTasks; }
        const auto& getBuildOrders() { return buildOrders; }
    private:
        vot::vector<vot::string> ptsUpdate;
        //vot::unordered_map<vot::string, Task> pts;
        vot::unordered_map<vot::string, PipeInfo> buildTasks;
        vot::vector<vot::string> buildOrders;
        std::unique_ptr<ShaderEditor> shaderEditor;
    };

} // ui

namespace yic{
    inline ui::ShaderHotReload* shaderHot;
}

#endif //VKCELSHADINGRENDERER_SHADERHOTRELOAD_H
