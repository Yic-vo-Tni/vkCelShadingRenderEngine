//
// Created by lenovo on 6/11/2024.
//

#include "vkImGui.h"

namespace yic {

    vkImGui::vkImGui() {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForVulkan(EventBus::Get::vkWindowContext().window.value(), false);

        ImGui_ImplVulkan_InitInfo info{
            .Instance = EventBus::Get::vkInitContext().instance.value(),
            .PhysicalDevice = EventBus::Get::vkDeviceContext().physicalDevice.value(),
            .Device = EventBus::Get::vkDeviceContext().device.value(),
            .Queue = EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsQueue(),
            .RenderPass = EventBus::Get::vkFrameRenderContext().renderPass.value(),
            .MinImageCount = static_cast<uint32_t>(EventBus::Get::vkSwapchainContext().frameEntries->size()),
            .ImageCount = static_cast<uint32_t>(EventBus::Get::vkSwapchainContext().frameEntries->size()),
        };
        ImGui_ImplVulkan_Init(&info);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    vkImGui::~vkImGui() {
        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

} // yic