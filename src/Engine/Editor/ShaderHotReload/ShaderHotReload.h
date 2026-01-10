//
// Created by lenovo on 6/10/2025.
//

#ifndef VKCELSHADINGRENDERER_SHADERHOTRELOAD_H
#define VKCELSHADINGRENDERER_SHADERHOTRELOAD_H

#include <complex>

#include "ShaderEditor.h"
#include "nlohmann/json.hpp"
#include "RHI/Pipeline/GraphicsPipeline.h"
#include "RHI/Pipeline/RayTracingPipeline.h"

namespace ui {

    inline auto normalizePath(const std::filesystem::path& p) -> std::string {
        auto abs = std::filesystem::absolute(p).generic_string();
        return abs;
    }

    struct ShaderCache {
        vot::unordered_map<vot::string, std::time_t> timestamps;

        auto load(const vot::string& file) -> void {
            if (!std::filesystem::exists(file)) return;
            std::ifstream in(file.c_str());
            nlohmann::json j;
            in >> j;
            for (auto& [p, t] : j.items()) {
                auto path = normalizePath(p);
                timestamps[path.data()] = t.get<std::time_t>();
            }
        }

        auto save(const vot::string& file) -> void {
            nlohmann::json j;
            for (auto& [p, t] : timestamps) {
                auto path = normalizePath(p);
                j[path] = t;
            }
            std::ofstream out(file.c_str());
            out << j.dump(4);
            out.close();
        }
    };

    class ShaderHotReload {
        struct PipeInfo{
            rhi::GraphicsPipeline* gp{};
            rhi::RayTracingPipeline* rp{};
            vk::ShaderStageFlagBits flags{};
        };
    public:
        MAKE_SINGLETON(ShaderHotReload);
        ShaderHotReload() {
            ShaderCache cache;
            cache.load(shader_path "shader_cache.json");
            checkShaderFilesIsUpdateOrNew(shader_path, cache);
            cache.save(shader_path "shader_cache.json");
            // compile();
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
            // std::string shaderPath = shader_path "/..";
            //
            // auto cmake_cmd = "cmake -S " + shaderPath + " -B " + shaderPath + "/build";
            // auto build_cmd = "cmake --build " + shaderPath + "/build";

            // boost::process::system(cmake_cmd, boost::process::std_out > stdout, boost::process::std_err > stderr);
            // boost::process::system(build_cmd, boost::process::std_out > stdout, boost::process::std_err > stderr);

            const std::string shaderPath = shader_path "/..";

            const auto cmake_cmd = "cmake -S \"" + shaderPath + "\" -B \"" + shaderPath + "/build\"";

            const auto build_cmd = "cmake --build \"" + shaderPath + "/build\"";

            run_command(cmake_cmd);
            run_command(build_cmd);
        }

        const auto& getBuildTasks() { return buildTasks; }
        const auto& getBuildOrders() { return buildOrders; }

    private:
        auto run_command(const std::string &cmd_utf8) -> bool {
            // UTF-8 -> UTF-16
            int wlen = MultiByteToWideChar(
                CP_UTF8,
                0,
                cmd_utf8.c_str(),
                -1,
                nullptr,
                0
            );

            std::wstring cmd(wlen, L'\0');

            MultiByteToWideChar(
                CP_UTF8,
                0,
                cmd_utf8.c_str(),
                -1,
                cmd.data(),
                wlen
            );

            STARTUPINFOW si{};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

            PROCESS_INFORMATION pi{};

            // 使用 cmd.exe /C，等价于 system()
            std::wstring full_cmd = L"cmd.exe /C " + cmd;

            BOOL ok = CreateProcessW(
                nullptr,
                full_cmd.data(),
                nullptr,
                nullptr,
                TRUE, // 继承 stdout/stderr
                0,
                nullptr,
                nullptr,
                &si,
                &pi
            );

            if (!ok) {
                return false;
            }

            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }


        auto checkShaderFilesIsUpdateOrNew(const vot::string& shaderDir, ShaderCache& cache) -> void {
            bool needCompile = false;
            for (auto& p : std::filesystem::recursive_directory_iterator(shaderDir)) {
                if (!p.is_regular_file()) continue;
                    if (!p.is_regular_file()) continue;
                    auto path = normalizePath(p.path());

                    if (path.find("shader_cache.json") != std::string::npos) continue;

                    auto ftime = std::filesystem::last_write_time(p);
                    auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
                    std::time_t lastWrite = std::chrono::system_clock::to_time_t(sctp);

                    if (!cache.timestamps.contains(path.data())) {
                        yic::logger->warn("new file");
                        needCompile = true;
                        cache.timestamps[path.data()] = lastWrite;
                    } else if (lastWrite > cache.timestamps[path.data()] + 1) {
                        yic::logger->warn("update file: {0}", cache.timestamps[path.data()]);
                        needCompile = true;
                        cache.timestamps[path.data()] = lastWrite;
                    }


            }
            if (needCompile) {
                yic::logger->warn("compile shader");
                compile();
            }
        }

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
