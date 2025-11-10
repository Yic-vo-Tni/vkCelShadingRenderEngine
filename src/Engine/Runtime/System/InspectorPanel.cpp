//
// Created by lenovo on 6/8/2025.
//

#include "InspectorPanel.h"

#include "RenderLibrary.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Editor/ImGuiHub.h"
#include "RS/ResourceSystem.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"
#include "Runtime/Camera/Camera.h"
#include "RHI/Command.h"
#include "Utils/Auxiliary.h"
#include "glm/gtx/matrix_decompose.hpp"

namespace sc {

    InspectorPanel::InspectorPanel(entt::registry& registry) : ecs(registry){}

    auto InspectorPanel::frame() -> void {
        yic::imguiHub->to(vot::uiWidget::ePanelWidget, [&] {
            yic::imguiHub->collapsingHeader("Atmosphere Effects", [&] {
                ImGui::Checkbox("Play Anims", &GLOBAL::playAllAnim);
                ImGui::Checkbox("Volumetric Clouds", &GLOBAL::showVolumetricClouds);
                ImGui::Checkbox("Volumetric Fog", &GLOBAL::showVolumetricFog);
            });

            yic::imguiHub->collapsingHeader("Lighting", [&] {
                static int selectedLightType = vot::comp::Light::Kind::eDirectional; // 0=Point, 1=Directional, 2=Spot


                if (ImGui::Button("Add Light")) {
                    //yic::systemHub.pub_enqueue(ev::tModelLoaded{});
                    // if (selectedLightType != 0
                    // auto e = ecs.create();
                    //
                    // switch (selectedLightType) {
                    //     case 0: // Point
                    //         ecs.emplace<vot::comp::Light::Point>(e,
                    //                                              glm::vec4(0.f, 2.f, 0.f, 1.f), // pos
                    //                                              glm::vec4(1.f, 1.f, 1.f, 1.f), // color
                    //                                              glm::vec4(1.f, 25.f, 1.f, 0.09f),
                    //                                              // intensity, radius, constant, linear
                    //                                              glm::vec4(0.032f, 0.f, 0.f, 0.f) // quadratic
                    //         );
                    //         break;
                    //     case 1: // Directional
                    //         ecs.emplace<vot::comp::Light::Directional>(e,
                    //                                                    glm::vec4(-0.3f, -1.f, -0.2f, 0.f), // dir
                    //                                                    glm::vec4(1.f, 1.f, 1.f, 1.f) // color
                    //         );
                    //         break;
                    //     case 2: // Spot
                    //         ecs.emplace<vot::comp::Light::Spot>(e,
                    //                                             glm::vec4(0.f, 2.f, 0.f, 1.f), // pos
                    //                                             glm::vec4(0.f, -1.f, 0.f, 0.f), // dir
                    //                                             glm::vec4(1.f, 1.f, 1.f, 1.f), // color
                    //                                             glm::vec4(glm::cos(glm::radians(12.5f)), // cutoff
                    //                                                       glm::cos(glm::radians(17.5f)), // outer cutoff
                    //                                                       0.f, 0.f)
                    //         );
                    //         break;
                    //     default: ;
                    // }
                }

                ImGui::SameLine();

                const char *lightTypes[] = {"Point Light", "Directional Light", "Spot Light"};
                ImGui::Combo("###Light Type", &selectedLightType, lightTypes, IM_ARRAYSIZE(lightTypes));
            });


            vot::vector<entt::entity> entities;
            ecs.view<const vot::mark::eVisible, vot::BasicInfoComponent, vot::AnimationComponent>().each(
                [&](const entt::entity e, vot::BasicInfoComponent &info, vot::AnimationComponent &ac) {
                    entities.emplace_back(e);
                });

            std::ranges::sort(entities, [&](const auto &a, const auto &b) {
                auto A = ecs.get<vot::AnimationComponent>(a).enableAnim;
                auto B = ecs.get<vot::AnimationComponent>(b).enableAnim;
                return A > B;
            });

            for (auto &e: entities) {
                auto &bic = ecs.get<vot::BasicInfoComponent>(e);
                auto &ac = ecs.get<vot::AnimationComponent>(e);
                const auto hideId = bic.name;

                yic::imguiHub->collapsingHeader(hideId.c_str(), [&] {
                    vot::scoped::ID id(hideId.c_str());

                    if_has<vot::mark::eMMD>(e,
                                            [&] { drawAnimComboForMMD(e, ac); },
                                            [&] { drawAnimComboForGeneric(ac); });
                    drawPlayButton(bic, ac);
                });
            }
        });


        yic::imguiHub->to(vot::uiWidget::eRenderWidget, [&] {
            mousePick();
            drawGizmo();
        });

        yic::imguiHub->to(vot::uiWidget::eViewWidget, [&] {
            drawModelList();
            drawShaderFileList();
        });

        // yic::imguiHub->to(vot::uiWidget::eNodeWidget, [&] {
        //     ImNodes::BeginNodeEditor();
        //
        //     ImNodes::BeginNode(1);
        //     ImGui::Dummy(ImVec2(80.0f, 45.0f));
        //     ImNodes::EndNode();
        //
        //     ImNodes::EndNodeEditor();
        // });
    }

    auto InspectorPanel::drawAnimComboForMMD(const entt::entity &entity, vot::AnimationComponent &ac) -> void {
        const auto& [vmd_name, vmd] = ac.vmd;
        const auto label = fmt::format("Select Animation##{}", static_cast<uint32_t>(entity));

        if (vot::scoped::Combo combo{label.c_str(), vmd_name.empty() ? "No Animations" : vmd_name.c_str()}) {
            for(const auto & vmdFile : yic::resourceSystem->mLoader->gVmdFiles()){
                if (ImGui::Selectable(vmdFile.first.c_str())){
                    yic::resourceSystem->mAnimator->bindVmd(vmdFile, ecs.get<vot::VertexDataComponent>(entity), ac);
                    if ( vmd)
                    yic::logger->info("bind vmd success");
                }
            }
        }
    }

    auto InspectorPanel::drawAnimComboForGeneric(vot::AnimationComponent &ac) -> void {
        auto& activeAnim = ac.activeAnim;
        const auto &anims = ac.animations;

        if (vot::scoped::Combo combo{"Select Animation", anims.empty() ? "No Animations" : anims[activeAnim].first.c_str()}){
            for (int i = 0; i < anims.size(); i++) {
                if (ImGui::Selectable(anims[i].first.c_str())) {
                    activeAnim = i;
                }
            }
        }
    }

    auto InspectorPanel::drawPlayButton(vot::BasicInfoComponent &info, vot::AnimationComponent& ac) -> void {
        const auto playAnimLabel = "PlayAnim###play_" + info.name;
        const auto enableAnimLabel = "EnableAnim###enable_" + info.name;

        ImGui::Checkbox(enableAnimLabel.c_str(), &ac.enableAnim);

        ImGui::SameLine();

        ImGui::Checkbox(playAnimLabel.c_str(), &info.playAnimation);
    }

    auto InspectorPanel::drawGizmo() -> void {
        ecs.view<vot::RenderComponent, const vot::BasicInfoComponent>().each([&](entt::entity, vot::RenderComponent& rc, const vot::BasicInfoComponent& bic) {
            if (GLOBAL::pickON == bic.name && GLOBAL::visibleZMO){
                const auto camera = GLOBAL::entity::camera.va<Camera>();
                auto view = camera.getView();
                auto proj = camera.getProj();
                proj[1][1] = -proj[1][1];

                const auto windowPos = ImGui::GetWindowPos();
                const auto windowSize = ImGui::GetWindowSize();

                const auto center = rc.center;
                const auto T = glm::translate(glm::mat4(1.f), center);
                const auto invT = glm::translate(glm::mat4(1.f), -center);

                auto temp = invT * rc.zmoMat * T;

                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
                ImGuizmo::Manipulate(glm::value_ptr(view),
                                     glm::value_ptr(proj),
                                     GLOBAL::gizmoOperation,
                                     ImGuizmo::MODE::LOCAL,
                                     glm::value_ptr(temp));

                // HACK: Zoom too small and the model will disappear

                rc.zmoMat = T * temp * invT;
            }
        });
    }

    auto InspectorPanel::drawModelList() -> void {
        yic::imguiHub->collapsingHeader("Models", [&] {
            ecs.view<const vot::BasicInfoComponent>().each([&](entt::entity e, const vot::BasicInfoComponent &bic) {
                if (ImGui::Selectable(bic.name.c_str())) {
                    GLOBAL::pickON = bic.name;
                }
            });
        });
    }

    auto InspectorPanel::drawShaderFileList() -> void {
        yic::imguiHub->collapsingHeader("Shader Files", [&] {
            for (auto &pt: yic::shaderHot->getBuildOrders()) {
                if (ImGui::Selectable(pt.c_str())) {
                    yic::shaderHot->tempEditor(pt);
                }
            }
        });
    }

    auto InspectorPanel::mousePick() -> void {
        const auto [u, v] = GLOBAL::mousePick;
        if (u < 0.f || v < 0.f) return;

        const auto extent = yic::renderLibrary->RT_IDBuffer->config.extent;
        const auto x = static_cast<int>(u * static_cast<float>(extent.width));
        const auto y = static_cast<int>(v * static_cast<float>(extent.height));

        const auto dev = yic::systemHub.va<ev::pVkSetupContext>().device;
        const auto physDev = yic::systemHub.va<ev::pVkSetupContext>().physicalDevice;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size  = sizeof(uint32_t);
        bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        const auto stagBuffer = dev->createBuffer(bufferInfo);

        const vk::MemoryRequirements memReq = dev->getBufferMemoryRequirements(stagBuffer);

        const vk::PhysicalDeviceMemoryProperties memProps = physDev->getMemoryProperties();
        uint32_t memoryTypeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if ((memReq.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags &
                (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))) {
                memoryTypeIndex = i;
                break;
                }
        }

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize  = memReq.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        const auto stagDeviceMem = dev->allocateMemory(allocInfo);

        dev->bindBufferMemory(stagBuffer, stagDeviceMem, 0);

        yic::command->drawOneTimeSubmit([&](const vot::CommandBuffer& cmd) {
            const auto region = vk::BufferImageCopy()
                .setBufferOffset(0)
                .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource(vk::ImageSubresourceLayers()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setMipLevel(0))
            .setImageOffset(vk::Offset3D(x, y, 0))
            .setImageExtent(vk::Extent3D(1, 1, 1));

            cmd.copyImageToBuffer(yic::renderLibrary->RT_IDBuffer->images, vk::ImageLayout::eTransferSrcOptimal, stagBuffer, region);
        });

        //auto entityID = 0u;
        uint32_t entityID = -1;
        const void* data = dev->mapMemory(stagDeviceMem, 0, sizeof(uint32_t));
        std::memcpy(&entityID, data, sizeof(uint32_t));
        dev->unmapMemory(stagDeviceMem);

        dev->destroy(stagBuffer);
        dev->free(stagDeviceMem);

        GLOBAL::mousePick = {-1.f, -1.f};

        yic::logger->warn(entityID);
        if (entityID != 0){
            const auto e = static_cast<entt::entity>(entityID);
            yic::logger->warn(entityID);
            if (ecs.valid(e)) {
                GLOBAL::pickON = ecs.get<vot::BasicInfoComponent>(e).name;
            }
        }
    }
} // sc