//
// Created by lenovo on 8/31/2025.
//

#ifndef VKCELSHADINGRENDERER_IMGUISCOPED_H
#define VKCELSHADINGRENDERER_IMGUISCOPED_H

namespace vot {
    namespace scoped {

        struct ID {
            explicit ID(const char* id) { ImGui::PushID(id); }
            ~ID() { ImGui::PopID(); }
        };

        struct Combo {
            Combo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0) {
                open = ImGui::BeginCombo(label, preview_value, flags);
            }
            ~Combo() {
                if (open) ImGui::EndCombo();
            }

            explicit operator bool() const { return open; }
            bool isOpen() const { return open; }

        private:
            bool open;
        };
    }
}

#endif //VKCELSHADINGRENDERER_IMGUISCOPED_H