//
// Created by lenovo on 1/22/2026.
//

#ifndef VKCELSHADINGRENDERER_PATH_H
#define VKCELSHADINGRENDERER_PATH_H

namespace vot::core::path {
    inline constexpr auto Shader = shader_path;
    inline constexpr auto ShaderSpv = spv_path;
    inline constexpr auto Texture = tex_path;
    inline constexpr auto ImGuiIni = imgui_ini_path;
    inline constexpr auto WebView = webview_path;

    template<typename T = container::string>
    inline auto join(const std::initializer_list<std::string_view> parts) -> T {
        T result;
        for (const auto p: parts) {
            result += p;
        }
        return result;
    }
}

#endif //VKCELSHADINGRENDERER_PATH_H