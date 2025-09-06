//
// Created by lenovo on 5/16/2025.
//

#ifndef VKCELSHADINGRENDERER_IMGUIHUB_H
#define VKCELSHADINGRENDERER_IMGUIHUB_H

namespace vot::inline ui{
    enum uiWidget : int {
        eRenderWidget, eViewWidget, ePanelWidget, eConsoleWidget, eCount
    };
//    enum uiAction{
//        eUpdateImage, eAddControl,
//    };
}

namespace ui {

    class ImGuiHub {
    public:
        Make = []{ return Singleton<ImGuiHub>::make_ptr(); };
        ImGuiHub() = default;
        ~ImGuiHub() = default;

        auto to(const vot::uiWidget& widget, const std::function<void()>& fn) -> void;
        auto bind(const vot::uiWidget& widget, const std::function<void()>& fn) -> void;
        auto exe(const vot::uiWidget& widget) -> void;
    public:
        auto collapsingHeader(const char *label, const std::function<void()> &fn,
                              ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed,
                              float indent = 5.f) -> void;
    private:
        std::array<oneapi::tbb::concurrent_queue<std::function<void()>>, vot::uiWidget::eCount> tasks{};
        std::unordered_map<vot::uiWidget, vot::vector<std::function<void()>>> pers{};
    };

} // ui

namespace yic{
    inline ui::ImGuiHub* imguiHub;
}

#endif //VKCELSHADINGRENDERER_IMGUIHUB_H
