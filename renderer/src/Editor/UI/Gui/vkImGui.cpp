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

        vk::DescriptorPoolSize poolSize[] = { {vk::DescriptorType::eCombinedImageSampler, 1}};
        auto pool_info = vk::DescriptorPoolCreateInfo().setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                                                                            .setMaxSets(1)
                                                                            .setPoolSizes(poolSize);
        mDescriptorPool = vkCreate("create imgui descriptor pool") = [&]{
            return EventBus::Get::vkDeviceContext().device.value().createDescriptorPool(pool_info);
        };



        ImGui_ImplVulkan_InitInfo info{
            .Instance = EventBus::Get::vkInitContext().instance.value(),
            .PhysicalDevice = EventBus::Get::vkDeviceContext().physicalDevice.value(),
            .Device = EventBus::Get::vkDeviceContext().device.value(),
            .Queue = EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsQueue(),
            .DescriptorPool = mDescriptorPool,
            .RenderPass = EventBus::Get::vkFrameRenderContext(et::vkFrameRenderContext::id::imguiFrameRender).renderPass.value(),
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

        EventBus::Get::vkDeviceContext().device.value().destroy(mDescriptorPool);
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