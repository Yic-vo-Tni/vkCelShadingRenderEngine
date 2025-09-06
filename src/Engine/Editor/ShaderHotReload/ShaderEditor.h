//
// Created by lenovo on 6/9/2025.
//

#ifndef VKCELSHADINGRENDERER_SHADEREDITOR_H
#define VKCELSHADINGRENDERER_SHADEREDITOR_H

namespace ui {

    class ShaderEditor {
    public:
        ShaderEditor();
        ~ShaderEditor();

        auto build(const vot::string& pt) -> void;

    private:
        auto openFile(const std::string& pt) -> std::string ;
        bool writeFile(const std::string& pt, const std::string& context);
//        auto switchFile(const std::string& pt) -> void;

    private:
        vot::string path;
    };

} // ui

#endif //VKCELSHADINGRENDERER_SHADEREDITOR_H
