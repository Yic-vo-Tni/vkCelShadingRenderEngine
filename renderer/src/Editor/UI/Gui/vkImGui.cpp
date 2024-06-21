//
// Created by lenovo on 6/11/2024.
//

#include "vkImGui.h"

#include <utility>

namespace yic {

    vkImGui::vkImGui(std::string id, vk::Queue graphicsQueue, const uint32_t &queueFamilyIndex) :
            mId(std::move(id)),
            mQueueIndex(queueFamilyIndex), mQueue(graphicsQueue),
            mWindow(EventBus::Get::vkRenderContext(mId).window_ref()){

        ImGui::CreateContext();
        auto &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForVulkan(mWindow, false);

        vk::DescriptorPoolSize poolSize[] = {{vk::DescriptorType::eCombinedImageSampler, 1}};
        auto pool_info = vk::DescriptorPoolCreateInfo().setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(1)
                .setPoolSizes(poolSize);
        mDescriptorPool = vkCreate("create imgui descriptor pool") = [&] {
            return EventBus::Get::vkSetupContext().device_ref().createDescriptorPool(pool_info);
        };

        auto ct = EventBus::Get::vkSetupContext();
        auto rt = EventBus::Get::vkRenderContext(mId);

        ImGui_ImplVulkan_InitInfo info{
                .Instance = ct.instance_ref(),
                .PhysicalDevice = ct.physicalDevice_ref(),
                .Device = ct.device_ref(),
                .QueueFamily = mQueueIndex,
                .Queue = mQueue,
                .DescriptorPool = mDescriptorPool,
                .RenderPass = rt.renderPass_ref(),
                .MinImageCount = rt.imageCount_v(),
                .ImageCount = rt.imageCount_v(),
        };
        ImGui_ImplVulkan_Init(&info);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    vkImGui::~vkImGui() {
        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        EventBus::Get::vkSetupContext().device_ref().destroy(mDescriptorPool);
    }

    auto vkImGui::render() -> void {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        callback();

        if (mShowDemo) ImGui::ShowDemoWindow(&mShowDemo);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), EventBus::Get::vkRenderContext(mId).cmd_ref());
    }

    auto vkImGui::callback() -> void{
        EventBus::subscribeDeferredAuto([&](const et::glKeyInput& input){
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

        EventBus::subscribeDeferredAuto([&](const et::glMouseInput& input){
            ImGui_ImplGlfw_MouseButtonCallback(mWindow, input.button.value(), input.action.value(), input.modes.value());
        });

        EventBus::subscribeDeferredAuto([&](const et::glCursorPosInput& input){
            ImGui_ImplGlfw_CursorPosCallback(mWindow, input.xpos.value(), input.ypos.value());
        });
        EventBus::subscribeDeferredAuto([&](const et::glScrollInput& input){
            ImGui_ImplGlfw_ScrollCallback(mWindow, input.xoffset.value(), input.yoffset.value());
        });
    }


} // yic