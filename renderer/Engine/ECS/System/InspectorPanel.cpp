//
// Created by lenovo on 6/8/2025.
//

#include "InspectorPanel.h"
#include "Editor/ImGuiHub.h"
#include "RS/ResourceSystem.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"
#include "ECS/Camera/Camera.h"
#include "Utils/Auxiliary.h"

namespace sc {

    InspectorPanel::InspectorPanel(entt::registry& registry) : ecs(registry){}
    //InspectorPanel::InspectorPanel(flecs::world &world) : ecs(world){}

    auto InspectorPanel::frame() -> void {
        yic::imguiHub->to(vot::uiWidget::ePanelWidget, [&]{
            //ecs.query<const vot::RenderVisibleTag, vot::BasicInfoComponent, vot::AnimationComponent>().each(
                    //[&](flecs::entity e, const vot::RenderVisibleTag,
            ecs.view<const vot::RenderVisibleTag, vot::BasicInfoComponent, vot::AnimationComponent>().each(
                    [&](entt::entity e,
                        vot::BasicInfoComponent &info,
                        vot::AnimationComponent &ac) {
                auto hideId = info.name;
                auto& activeAnim = ac.activeAnim;
                const auto& anims = ac.animations;
                const auto& vmd = ac.vmd;

                yic::imguiHub->collapsingHeader(hideId.c_str(), [&]{
                    ImGui::PushID(hideId.c_str());

                    //if (e.has<vot::MMDTag>()){
                    if (ecs.all_of<vot::MMDTag>(e)){
                        if (ImGui::BeginCombo("Select Animation", vmd.first.empty() ? "No Animations" : vmd.first.c_str())){
                            for(const auto & ptVmd : yic::resourceSystem->mLoader->mMmdLoader->ptVmds){
                                if (ImGui::Selectable(ptVmd.c_str())){
                                  //  yic::resourceSystem->mLoader->mMmdLoader->bindVmd(ptVmd, *e.get<vot::VertexDataComponent>(), ac);
                                    yic::resourceSystem->mLoader->mMmdLoader->bindVmd(ptVmd, ecs.get<vot::VertexDataComponent>(e), ac);
//                                    yic::resourceSystem->mLoader->mMmdLoader->bakeVmd(ecs.get<vot::VertexDataComponent>(e), ac);
                                    yic::logger->info("bind vmd success");
                                   // activeAnim = 0;
                                }
                            }
                            ImGui::EndCombo();
                        }
                    } else {
                        if (ImGui::BeginCombo("Select Animation", anims.empty() ? "No Animations" : anims[activeAnim].first.c_str())) {
                            for (auto i = 0; i < anims.size(); i++) {
                                if (ImGui::Selectable(anims[i].first.c_str())) {
                                    activeAnim = i;
                                }
                            }
                            ImGui::EndCombo();
                        }
                    }

                    if(ImGui::Button("play")){
                        info.playAnimation = !info.playAnimation;
                    };

                    ImGui::PopID();
                });

                if (ImGui::IsItemHovered()){

                }
            });
        });

        yic::imguiHub->to(vot::uiWidget::eRenderWidget, [&]{
            //ecs.query<vot::RenderComponent, const vot::BasicInfoComponent>().each([&](flecs::entity_view e, vot::RenderComponent& rc, const vot::BasicInfoComponent& bic){
            ecs.view<vot::RenderComponent, const vot::BasicInfoComponent>().each([&](entt::entity e, vot::RenderComponent& rc, const vot::BasicInfoComponent& bic){
                if (GLOBAL::pickON == bic.name && GLOBAL::visibleZMO){
                    //auto camera = sc::camera_comp(ecs);
                    auto camera = ecs.get<sc::Camera>(GLOBAL::camera);
                    auto view = camera.getView();
                    auto proj = camera.getProj();
//                    auto view = camera->getView();
//                    auto proj = camera->getProj();
                    proj[1][1] = -proj[1][1];

                    auto windowPos = ImGui::GetWindowPos();
                    auto windowSize = ImGui::GetWindowSize();

                    auto center = rc.center;
                    auto T = glm::translate(glm::mat4(1.f), center);
                    auto invT = glm::translate(glm::mat4(1.f), -center);

                    auto temp = invT * rc.zmoMat * T;

                    ImGuizmo::SetDrawlist();
                    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
                    ImGuizmo::Manipulate(glm::value_ptr(view),
                                         glm::value_ptr(proj),
                                         GLOBAL::gizmoOperation,
                                         ImGuizmo::MODE::LOCAL,
                                         glm::value_ptr(temp)
                    );

                    rc.zmoMat = T * temp * invT;
                }
            });

        });

        yic::imguiHub->to(vot::uiWidget::eViewWidget, [&]{
            yic::imguiHub->collapsingHeader("Models", [&]{
               // ecs.query<const vot::BasicInfoComponent>().each([&](flecs::entity_view e, const vot::BasicInfoComponent& bic){
                ecs.view<const vot::BasicInfoComponent>().each([&](entt::entity e, const vot::BasicInfoComponent& bic){
                    if (ImGui::Selectable(bic.name.c_str())){
                        GLOBAL::pickON = bic.name;
                    }
                });
            });

            yic::imguiHub->collapsingHeader("Shader Files", [&]{
                for (auto &pt : yic::shaderHot->getBuildOrders()) {
                    if (ImGui::Selectable(pt.c_str())) {
                        yic::shaderHot->tempEditor(pt);
                    }
                }
            });
        });

    }

} // sc