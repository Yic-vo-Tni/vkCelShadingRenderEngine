//
// Created by lenovo on 6/11/2024.
//

#include "vkImGui.h"

namespace yic {

    vkImGui::vkImGui() {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForVulkan(get<GLFWwindow*>(EventBus::Get::vkRenderContext().window_v()), false);

        vk::DescriptorPoolSize poolSize[] = { {vk::DescriptorType::eCombinedImageSampler, 1}};
        auto pool_info = vk::DescriptorPoolCreateInfo().setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                                                                            .setMaxSets(1)
                                                                            .setPoolSizes(poolSize);
        mDescriptorPool = vkCreate("create imgui descriptor pool") = [&]{
            return EventBus::Get::vkSetupContext().device_v().createDescriptorPool(pool_info);
        };



        ImGui_ImplVulkan_InitInfo info{
            .Instance = EventBus::Get::vkSetupContext().instance_v(),
            .PhysicalDevice = EventBus::Get::vkSetupContext().physicalDevice_v(),
            .Device = EventBus::Get::vkSetupContext().device_v(),
            .QueueFamily = EventBus::Get::vkSetupContext().qIndex_imGuiGraphics_v(),
            .Queue = EventBus::Get::vkSetupContext().queue_imGuiGraphics_v(),
            .DescriptorPool = mDescriptorPool,
            .RenderPass = EventBus::Get::vkRenderContext().renderPass_v(),
            .MinImageCount = EventBus::Get::vkRenderContext().imageCount_v(),
            .ImageCount = EventBus::Get::vkRenderContext().imageCount_v(),
        };
        ImGui_ImplVulkan_Init(&info);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    vkImGui::~vkImGui() {
        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        EventBus::Get::vkSetupContext().device_v().destroy(mDescriptorPool);
    }

    auto vkImGui::beginRenderImGui() -> void {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (mShowDemo) ImGui::ShowDemoWindow(&mShowDemo);
    }

    auto vkImGui::endRenderImGui(const vk::CommandBuffer &cmd) -> void {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

} // yic