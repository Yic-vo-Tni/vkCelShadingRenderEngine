//
// Created by lenovo on 10/3/2024.
//

#include "ImGui.h"
#include "RHI/QueueFamily.h"
#include "Core/DispatchSystem/SystemHub.h"

#include "Widget/RenderWidget.h"
#include "Widget/ViewWidget.h"
#include "Widget/ConsoleWidget.h"
#include "Widget/PanelWidget.h"

#include "Window.h"
#include "ImGuiHub.h"

namespace ui {

    ImGuiLauncher ::ImGuiLauncher () {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = imgui_ini_path "ImGui.ini";
        const char* fontPt = R"(H:\VkCelShadingRenderer\renderer\resource\TTF\JetBrainsMono-Regular.ttf)";
        io.Fonts->AddFontFromFileTTF(fontPt, 18.f);

        auto wt = yic::systemHub.val<ev::pWindowContext>();
        auto ct = yic::systemHub.val<ev::pVkSetupContext>();
        auto rt = yic::systemHub.val<ev::pVkRenderContext>();
        mWindow = wt.window;

        mCurrentExtent = rt.currentExtent;
        ImGui_ImplGlfw_InitForVulkan(wt.window, false);

        auto poolInfo = vk::DescriptorPoolCreateInfo()
                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(1)
                .setPoolSizes(vk::DescriptorPoolSize()
                .setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(5));
        mDescriptorPool = vot::create("create gui des pool") = [&]{
            return ct.device->createDescriptorPool(poolInfo);
        };

        VkFormat colorFormat[] = {static_cast<VkFormat>(rt.surfaceFormat->format)};

        ImGui_ImplVulkan_InitInfo info{
            .Instance = *ct.instance,
            .PhysicalDevice = *ct.physicalDevice,
            .Device = *ct.device,
            .QueueFamily = yic::qFamily->acquireQueueIndex(vot::queueType::eGraphics),
            .Queue = yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics),
            .DescriptorPool = mDescriptorPool,
            .MinImageCount = (uint32_t)rt.frameEntries->size(),
            .ImageCount = (uint32_t)rt.frameEntries->size(),
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                    .colorAttachmentCount = 1,
                    .pColorAttachmentFormats = colorFormat
            }
        };
        ImGui_ImplVulkan_Init(&info);

        callback(wt.window);

        ImGui_ImplVulkan_CreateFontsTexture();

        mWidgets.emplace_back(std::move(std::make_unique<RenderWidget>()));
        mWidgets.emplace_back(std::move(std::make_unique<ConsoleWidget>()));
        mWidgets.emplace_back(std::move(std::make_unique<ViewWidget>()));
        mWidgets.emplace_back(std::move(std::make_unique<PanelWidget>()));

        yic::imguiHub = ImGuiHub::make();
    }

    ImGuiLauncher::~ImGuiLauncher() {
        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        mWidgets.clear();
        yic::systemHub.val<ev::pVkSetupContext>().device->destroy(mDescriptorPool);
    }

    auto ImGuiLauncher::draw(vk::CommandBuffer &cmd) -> void {
 //       auto window = yic::systemHub.val<ev::pWindowContext>().window;

        {
            auto readIndex = yic::glT::keyInputActive.load(std::memory_order_acquire);
            auto& keyInput = yic::glTBuffers[readIndex].keyInput;
            ImGui_ImplGlfw_KeyCallback(mWindow, keyInput.key, keyInput.scancode, keyInput.action, keyInput.mods);
        }
        {
            auto readIndex = yic::glT::mouseInputActive.load(std::memory_order_acquire);
            auto& mouseInput = yic::glTBuffers[readIndex].mouseInput;
            ImGui_ImplGlfw_MouseButtonCallback(mWindow, mouseInput.button, mouseInput.action, mouseInput.mods);
        }
        {
            auto readIndex = yic::glT::cursorPosInputActive.load(std::memory_order_acquire);
            auto& cursorPosInput = yic::glTBuffers[readIndex].cursorPosInput;
            ImGui_ImplGlfw_CursorPosCallback(mWindow, cursorPosInput.xpos, cursorPosInput.ypos);
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        //ImGui::ShowDemoWindow(&mShowDemo);

        base();

        for(auto& w : mWidgets){
            ImGui::Begin(w->getName().c_str());
            w->rec();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    auto ImGuiLauncher::base() -> void {
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
       // colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
        colors[ImGuiCol_WindowBg] = ImVec4{0.12f, 0.12f, 0.14f, 1.0f};
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

    auto ImGuiLauncher::callback(GLFWwindow* window) -> void {
        yic::systemHub.subscribe([&](const ev::glKeyInput& input){

        });

        yic::systemHub.subscribe([&](const ev::glMouseInput& input){

        });

        yic::systemHub.subscribe([&](const ev::glCursorPosInput& input){

        });
        yic::systemHub.subscribe([&](const ev::glScrollInput& input){

        });
    }

    auto ImGuiLauncher::updateSwap() -> void {
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - lastSwapTime >= swapInterval){
            bSwap = !bSwap;
            lastSwapTime = currentTime;
        }
    }

} // ui