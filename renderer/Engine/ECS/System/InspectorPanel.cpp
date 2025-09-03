//
// Created by lenovo on 6/8/2025.
//

#include "InspectorPanel.h"

#include "RenderLibrary.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Editor/ImGuiHub.h"
#include "RS/ResourceSystem.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"
#include "ECS/Camera/Camera.h"
#include "RHI/Command.h"
#include "Utils/Auxiliary.h"

namespace sc {

    InspectorPanel::InspectorPanel(entt::registry& registry) : ecs(registry){}

    auto InspectorPanel::frame() -> void {
        yic::imguiHub->to(vot::uiWidget::ePanelWidget, [&] {
            yic::imguiHub->collapsingHeader("Atmosphere Effects", [&] {
                ImGui::Checkbox("Play Anims", &GLOBAL::playAllAnim);
                ImGui::Checkbox("Volumetric Clouds", &GLOBAL::showVolumetricClouds);
                ImGui::Checkbox("Volumetric Fog", &GLOBAL::showVolumetricFog);
            });

        ecs.view<const vot::mark::eVisible, vot::BasicInfoComponent, vot::AnimationComponent>().each(
            [&](const entt::entity e, vot::BasicInfoComponent &info, vot::AnimationComponent &ac) {
                const auto hideId = info.name;

                yic::imguiHub->collapsingHeader(hideId.c_str(), [&] {
                    vot::scoped::ID id(hideId.c_str());

                    if_has<vot::mark::eMMD>(e,
                                            [&] { drawAnimComboForMMD(e, ac); },
                                            [&] { drawAnimComboForGeneric(ac); });

                    drawPlayButton(info, ac);
                });
            });
        });

        yic::imguiHub->to(vot::uiWidget::eRenderWidget, [&] {
            mousePick();
            drawGizmo();
        });

        yic::imguiHub->to(vot::uiWidget::eViewWidget, [&] {
            drawModelList();
            drawShaderFileList();
        });
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
        const auto playAnimLabel = "play###play_" + info.name;
        const auto enableAnimLabel = "enableAnim###enable_" + info.name;

        ImGui::Checkbox(enableAnimLabel.c_str(), &ac.enableAnim);

        ImGui::SameLine();

        if(ImGui::Button(playAnimLabel.c_str())){
            info.playAnimation = !info.playAnimation;
        }
    }

    auto InspectorPanel::drawGizmo() -> void {
        ecs.view<vot::RenderComponent, const vot::BasicInfoComponent>().each([&](entt::entity, vot::RenderComponent& rc, const vot::BasicInfoComponent& bic) {
            if (GLOBAL::pickON == bic.name && GLOBAL::visibleZMO){
                const auto camera = ecs.get<sc::Camera>(GLOBAL::camera);
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
                                     glm::value_ptr(temp)
                );

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
        auto x = static_cast<int>(u * extent.width);
        auto y = static_cast<int>(v * extent.height);

        auto dev = yic::systemHub.va<ev::pVkSetupContext>().device;
        auto physDev = yic::systemHub.va<ev::pVkSetupContext>().physicalDevice;

        vk::Buffer stagBuffer;
        vk::DeviceMemory stagDeviceMem;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size  = sizeof(uint32_t);
        bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        stagBuffer = dev->createBuffer(bufferInfo);

        vk::MemoryRequirements memReq = dev->getBufferMemoryRequirements(stagBuffer);

        vk::PhysicalDeviceMemoryProperties memProps = physDev->getMemoryProperties();
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

        stagDeviceMem = dev->allocateMemory(allocInfo);

        dev->bindBufferMemory(stagBuffer, stagDeviceMem, 0);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd) {
            auto region = vk::BufferImageCopy()
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

        auto entityID = 0u;
        const void* data = dev->mapMemory(stagDeviceMem, 0, sizeof(uint32_t));
        std::memcpy(&entityID, data, sizeof(uint32_t));
        dev->unmapMemory(stagDeviceMem);

        dev->destroy(stagBuffer);
        dev->free(stagDeviceMem);

        GLOBAL::mousePick = {-1.f, -1.f};

        if (entityID != 0){
            auto e = static_cast<entt::entity>(entityID);
            yic::logger->warn(entityID);
            if (ecs.valid(e)) {
                GLOBAL::pickON = ecs.get<vot::BasicInfoComponent>(e).name;
            }
        }
    }
} // sc