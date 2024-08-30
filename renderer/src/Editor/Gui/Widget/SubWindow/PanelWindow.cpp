//
// Created by lenovo on 8/5/2024.
//

#include "PanelWindow.h"

namespace ui {


    void PanelWindow::record() {
        shaderFilePaths();
    }

    auto PanelWindow::shaderFilePaths() -> void {

        std::vector<std::string> shaderPts;

        auto ecs = yic::EventBus::val<et::eEcs_ptr>().ecs_ref();

//        ecs->query<sc::Model::Generic, Selected>().each([&](flecs::entity e, sc::Model::Generic& model, Selected selected){
//            shaderPts.insert(shaderPts.end(), model.shaderPaths.begin(), model.shaderPaths.end());
//        });
//
//        ecs->query<sc::Model::Pmx, Selected>().each([&](flecs::entity e, sc::Model::Pmx& pmx, Selected selected){
//            if(ImGui::Button("Load Vmd Animation")){
//                EventBus::publishAsync(et::eResPathsNfd{std::make_pair(ResFormat::eVmd, &pmx)});
//
//            }
//
//            shaderPts.insert(shaderPts.end(), pmx.shaderPaths.begin(), pmx.shaderPaths.end());
//        });
//
//        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
//        for(const auto& pt : shaderPts){
//            if(ImGui::Selectable(pt.c_str(), false, ImGuiSelectableFlags_DontClosePopups)){
//                auto wholePt = shader_path + std::string ("/") + pt;
//                ShaderEditor::openShaderFile(wholePt);
//            }
//        }
//        ImGui::PopStyleColor(1);


















//        if (EventBus::val<et::eModel_ptr>().model.has_value()) {
//            auto model = EventBus::val<et::eModel_ptr>().model_ref();
//
//            if (model->format == ResFormat::ePmx){
//                nfdchar_t *outPath{};
//                auto r = NFD_OpenDialog("vmd", nullptr, &outPath);
//                if (r == NFD_OKAY){
//                    std::unordered_map<ResFormat, std::vector<std::string>> pts;
//                    pts[ResFormat::eVmd].emplace_back(outPath);
//                    EventBus::publishAsync(et::eResPaths{pts});
//                }
//            }
//
//            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
//            for(const auto& pt : model->shaderPaths){
//                if (ImGui::Selectable(pt.c_str(), false,
//                                      ImGuiSelectableFlags_DontClosePopups)) {
//                    auto totalPt = shader_path + std::string ("/") + pt;
//                    ShaderEditor::openShaderFile(totalPt);
//                }
//            }
//            ImGui::PopStyleColor(1);
//        }


        //ImGui::OpenPopup("My Popup");

//        if (ImGui::BeginPopupModal("My Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
//            ImGui::Separator();
//
//            static bool enable_vmd = false;
//            ImGui::Checkbox("启用 VMD 动画", &enable_vmd);
//
//            if (ImGui::Button("OK", ImVec2(120, 0))) {
//                if (enable_vmd) {
//                }
//                ImGui::CloseCurrentPopup();
//            }
//            ImGui::SetItemDefaultFocus();
//            ImGui::SameLine();
//            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
//                ImGui::CloseCurrentPopup();
//            }
//            ImGui::EndPopup();
//        }
    }


} // ui