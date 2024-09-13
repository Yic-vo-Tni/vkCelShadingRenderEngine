//
// Created by lenovo on 6/11/2024.
//

#include "vkImGui.h"

#include <utility>

namespace yic {

    vkImGui::vkImGui(vot::string id, vk::Queue graphicsQueue, const uint32_t &queueFamilyIndex) :
            mId(std::move(id)),
            mQueueIndex(queueFamilyIndex), mQueue(graphicsQueue),
            mWindow(mg::SystemHub.val<ev::hVkRenderContext>(mId).window){

        ImGui::CreateContext();
        auto &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::GetIO().IniFilename = imgui_ini_path "imgui.ini";

        ImGui_ImplGlfw_InitForVulkan(mWindow, false);

        auto ct = mg::SystemHub.val<ev::pVkSetupContext>();
        auto rt = mg::SystemHub.val<ev::hVkRenderContext>(mId);

        vk::DescriptorPoolSize poolSize[] = {{vk::DescriptorType::eCombinedImageSampler, 1}};
        auto pool_info = vk::DescriptorPoolCreateInfo().setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(1)
                .setPoolSizes(poolSize);
        mDescriptorPool = vkCreate("create imgui descriptor pool") = [&] {
            return ct.device->createDescriptorPool(pool_info);
        };

        ImGui_ImplVulkan_InitInfo info{
                .Instance = *ct.instance,
                .PhysicalDevice = *ct.physicalDevice,
                .Device = *ct.device,
                .QueueFamily = mQueueIndex,
                .Queue = mQueue,
                .DescriptorPool = mDescriptorPool,
                .RenderPass = *rt.renderPass,
                .MinImageCount = static_cast<uint32_t>(rt.framebuffers->size()),
                .ImageCount = static_cast<uint32_t>(rt.framebuffers->size()),
        };
        ImGui_ImplVulkan_Init(&info);

        callback();

        ImGui_ImplVulkan_CreateFontsTexture();

        mCurrentExtent = rt.currentExtent;
    }

    vkImGui::~vkImGui() {
        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        mg::SystemHub.val<ev::pVkSetupContext>().device->destroy(mDescriptorPool);
    }

    auto vkImGui::render(vk::CommandBuffer& cmd) -> void {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        base();

        mWidgetManager.render();

        //if (mShowDemo) ImGui::ShowDemoWindow(&mShowDemo);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    auto vkImGui::base() -> void {
        if (mCurrentExtent->width != mExtent.width || mCurrentExtent->height != mExtent.width) {
            ImVec2 imguiWindowSize((float) mCurrentExtent->width, (float) mCurrentExtent->height);
            ImVec2 imguiWindowPos(0, 0);
            ImGui::SetNextWindowSize(imguiWindowSize);
            ImGui::SetNextWindowPos(imguiWindowPos);
            mExtent = *mCurrentExtent;
        }

        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg].w = 0.f;
        style.WindowPadding = ImVec2(0.f, 0.f);
        style.WindowBorderSize = 0.f;
        style.FramePadding = ImVec2(15.f, 15.f);
        style.ItemSpacing = ImVec2(20.f, 20.f);
        style.TabRounding = 4;
        style.ScrollbarRounding = 9;
        style.WindowRounding = 7;
        style.GrabRounding = 3;
        style.FrameRounding = 3;
        style.PopupRounding = 4;

        style.FrameBorderSize = 0.f;

        auto &colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
        colors[ImGuiCol_MenuBarBg] = ImVec4{0.06f, 0.06f, 0.11f, 1.0f};

        colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
        colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};

        colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
        colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

        colors[ImGuiCol_Header] = ImVec4{0.33f, 0.33f, 0.47, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

        colors[ImGuiCol_Button] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_ButtonActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_CheckMark] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};

        colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

        colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
        colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

        colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13, 0.17, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

        colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TabHovered] = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
        colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
        colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

        colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

        colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
        colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

        colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
        colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
        colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

        colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
        colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
        colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

        colors[ImGuiCol_DockingPreview] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};


        ImGui::Begin("Yicvot", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar);
//        ImGui::Begin("Yicvot");
        {
            if (ImGui::BeginMenuBar()){
                if (ImGui::BeginMenu("File")) {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
                    if (ImGui::MenuItem("##e")){}
                    ImGui::PopStyleVar();

                    if (ImGui::MenuItem("open project")) {}

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
                    if (ImGui::MenuItem("##e")){}
                    ImGui::PopStyleVar();
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id);
        }
        ImGui::End();
    }

    auto vkImGui::callback() -> void{
        mg::SystemHub.subscribe([&](const et::glKeyInput& input){
            auto& io = ImGui::GetIO();
            switch (input.action.value()) {
                case GLFW_PRESS:
                    io.KeysDown[input.key.value()] = true;
                    break;
                case GLFW_RELEASE:
                    io.KeysDown[input.key.value()] = false;
                    break;
                default:
                    break;
            }
            io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
            io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
            io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
            io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

            ImGui_ImplGlfw_KeyCallback(mWindow, input.key.value(), input.scancode.value(), input.action.value(), input.mods.value());
        });

        mg::SystemHub.subscribe([&](const et::glMouseInput& input){
            ImGui_ImplGlfw_MouseButtonCallback(mWindow, input.button.value(), input.action.value(), input.modes.value());
        });

        mg::SystemHub.subscribe([&](const et::glCursorPosInput& input){
            ImGui_ImplGlfw_CursorPosCallback(mWindow, input.xpos.value(), input.ypos.value());
        });
        mg::SystemHub.subscribe([&](const et::glScrollInput& input){
            ImGui_ImplGlfw_ScrollCallback(mWindow, input.xoffset.value(), input.yoffset.value());
        });
    }


} // yic