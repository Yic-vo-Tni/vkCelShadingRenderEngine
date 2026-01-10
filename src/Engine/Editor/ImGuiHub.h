//
// Created by lenovo on 5/16/2025.
//

#ifndef VKCELSHADINGRENDERER_IMGUIHUB_H
#define VKCELSHADINGRENDERER_IMGUIHUB_H

namespace vot::inline ui{
    enum uiWidget : int {
        eRenderWidget, eViewWidget, ePanelWidget, eConsoleWidget, eNodeWidget, eDERWidget, eCount
    };
//    enum uiAction{
//        eUpdateImage, eAddControl,
//    };
}

namespace ui {

    struct WidgetSlot {
        // std::function<void()> bind;
        // vot::vector<std::function<void()>> fixed;
        // oneapi::tbb::concurrent_queue<std::function<void()>> tasks;
        std::move_only_function<void()> bind;
        vot::vector<std::move_only_function<void()>> fixed;
        oneapi::tbb::concurrent_queue<std::function<void()>> tasks;
    };

    class ImGuiHub {
    public:
        Make = []{ return Singleton<ImGuiHub>::make_ptr(); };
        ImGuiHub() = default;
        ~ImGuiHub() = default;

        auto to(const vot::uiWidget& widget, const std::function<void()>& fn) -> void;
        auto to_fixed(const vot::uiWidget& widget, const std::function<void()>& fn) -> void;
        auto bind(const vot::uiWidget& widget, const std::function<void()>& fn) -> void;
        auto exe(const vot::uiWidget& widget) -> void;
    public:
        auto collapsingHeader(const char *label, const std::function<void()> &fn,
                              ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed,
                              float indent = 5.f) -> void;
        auto button(const char* label, const std::function<void()> &fn) -> void;
    private:
        // std::array<oneapi::tbb::concurrent_queue<std::function<void()>>, vot::uiWidget::eCount> tasks{};
        // std::unordered_map<vot::uiWidget, vot::vector<std::function<void()>>> pers{};
        // std::unordered_map<vot::uiWidget, std::function<void()>> binds{};
        std::array<WidgetSlot, static_cast<size_t>(vot::uiWidget::eCount)> slots{};
    };



} // ui

namespace yic{
    inline ui::ImGuiHub* imguiHub;

    // namespace ui {
    //     struct UIContext {
    //         ::ui::ImGuiHub* hub;
    //         vot::uiWidget widget;
    //     };
    //
    //     namespace detail {
    //         struct imgui_t{};
    //
    //         struct panel_t{ vot::uiWidget widget; };
    //         struct collapsing_t{ const char* label; };
    //         struct button_t {
    //             const char* label;
    //             std::function<void()> fn;
    //         };
    //
    //         template<typename F>
    //         button_t operator>>(button_t b, F&& f) {
    //             b.fn = std::forward<F>(f);
    //             return b;
    //         }
    //
    //         inline UIContext operator|(imgui_t, panel_t p) {
    //             return UIContext{
    //                 .hub = imguiHub,
    //                 .widget = p.widget
    //             };
    //         }
    //
    //         inline UIContext operator|(const UIContext ctx, const collapsing_t c) {
    //             ctx.hub->collapsingHeader(c.label, []{});
    //             return ctx;
    //         }
    //
    //         inline UIContext operator|(const UIContext ctx, const button_t b) {
    //             if (b.fn) {
    //                 ctx.hub->button(b.label, b.fn);
    //             } else {
    //                 ctx.hub->button(b.label, []{});
    //             }
    //             return ctx;
    //         }
    //     }
    //
    //     inline constexpr detail::imgui_t imgui{};
    //
    //     inline auto panel(vot::uiWidget w) {
    //         return detail::panel_t{w};
    //     }
    //
    //     inline auto collapsing(const char* label) {
    //         return detail::collapsing_t{label};
    //     }
    //
    //     inline auto button(const char* label) {
    //         return detail::button_t{label, {}};
    //     }
    // }
}

#endif //VKCELSHADINGRENDERER_IMGUIHUB_H
