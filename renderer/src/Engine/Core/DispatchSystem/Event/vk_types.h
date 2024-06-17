//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKEVENTTYPES_H
#define VKCELSHADINGRENDERER_VKEVENTTYPES_H

namespace et {

    struct FrameEntry{
        vk::Image image;
        vk::ImageView imageView;
        vk::Semaphore readSemaphore;
        vk::Semaphore writtenSemaphore;
    };

    struct vkSetupContext{
        struct_y(vkSetupContext,
                 (opt<vk::Instance>, instance),
                 (opt<vk::DispatchLoaderDynamic>, dynamicDispatcher),
                 (opt<vk::DebugUtilsMessengerEXT>, debugMessenger),
                 (opt<vk::PhysicalDevice>, physicalDevice),
                 (opt<vk::Device>, device),
                 (opt<yic::QueueFamily>, queueFamily));

        [[nodiscard]] const auto& instance_v() const{
            if (!instance.has_value())
                throw std::runtime_error("instance is not initialized.");
            return instance.value();
        }
        [[nodiscard]] const auto& dynamicDispatcher_v() const{
            if (!dynamicDispatcher.has_value())
                throw std::runtime_error("dynamicDispatcher is not initialized.");
            return dynamicDispatcher.value();
        }
        [[nodiscard]] const auto& debugMessenger_v() const{
            if (!debugMessenger.has_value())
                throw std::runtime_error("debugMessenger is not initialized.");
            return debugMessenger.value();
        }
        [[nodiscard]] const auto& physicalDevice_v() const{
            if (!physicalDevice.has_value())
                throw std::runtime_error("physicalDevice is not initialized.");
            return physicalDevice.value();
        }
        [[nodiscard]] const auto& device_v() const{
            if (!device.has_value())
                throw std::runtime_error("device is not initialized.");
            return device.value();
        }
        [[nodiscard]] const auto& queue_imGuiGraphics_v() const  {
            if (!queueFamily.has_value())
                throw std::runtime_error("queueFamily is not initialized.");
            return queueFamily->getImGuiGraphicsQueue();
        }
        [[nodiscard]] auto qIndex_imGuiGraphics_v() const {
            if (!queueFamily.has_value())
                throw std::runtime_error("queueFamily is not initialized.");
            return queueFamily->getImGuiGraphicsFamilyIndex();
        }
        [[nodiscard]] const auto& queue_mainGraphics_v() const {
            if (!queueFamily.has_value())
                throw std::runtime_error("queueFamily is not initialized.");
            return queueFamily->getMainGraphicsQueue();
        }
        [[nodiscard]] auto qIndex_mainGraphics_v() const {
            if (!queueFamily.has_value())
                throw std::runtime_error("queueFamily is not initialized.");
            return queueFamily->getMainGraphicsFamilyIndex();
        }
    };


    struct vkRenderContext {
        struct_y(vkRenderContext,
                 (opt<std::pair<int, int>>, size),
                 (opt<vk::Extent2D>, extent),
                 (opt<std::variant<GLFWwindow*, HWND>>, window),
                 (opt<vk::SwapchainKHR>, swapchain),
                 (opt<std::vector<FrameEntry>>, frameEntries),
                 (opt<vk::SurfaceFormatKHR>, surfaceFormat),
                 (opt<uint32_t>, activeImageIndex),
                 (opt<vk::RenderPass>, renderPass),
                 (opt<std::vector<vk::Framebuffer>>, framebuffers),
                 (opt<std::unordered_map<std::string, vk::RenderPass>>, offscreenRenderPass),
                 (opt<std::unordered_map<std::string, std::vector<vk::Framebuffer>>>, offscreenFramebuffers)
        );

        struct id{
            static constexpr const char* imguiRender{"imguiRender"};
            static constexpr const char* mainRender{"mainRender"};
        };

        template<typename T>
        [[nodiscard]] const auto& window_v() const {
            if (!window.has_value())
                throw std::runtime_error("window is not initialized.");
            return std::get<T>(window.value());
        }

        [[nodiscard]] const auto& window_v() const{
            if (!window.has_value())
                throw std::runtime_error("window is not initialized.");
            return window.value();
        }

        [[nodiscard]] auto width_v() const {
            if (!size.has_value())
                throw std::runtime_error("window size is not initialized.");

            return size.value().first;
        }
        [[nodiscard]] auto height_v() const{
            if (!size.has_value())
                throw std::runtime_error("window size is not initialized.");

            return size.value().second;
        }
        [[nodiscard]] auto extent_v() const{
            if (!extent.has_value())
                throw std::runtime_error("extent is not initialized.");
            return extent.value();
        }

        [[nodiscard]] const auto& swapchain_v() const {
            if (!swapchain.has_value())
                throw std::runtime_error("Swapchain is not initialized.");
            return swapchain.value();
        }
        [[nodiscard]] const auto& frameEntries_v() const{
            if (!frameEntries.has_value())
                throw std::runtime_error("Frame entries are not initialized.");
            return frameEntries.value();
        }
        [[nodiscard]] auto imageCount_v() const{
            if (!frameEntries.has_value())
                throw std::runtime_error("Frame entries are not initialized -- image count.");
            return static_cast<uint32_t>(frameEntries->size());
        }
        [[nodiscard]] const auto& surfaceFormat_v() const{
            if (!surfaceFormat.has_value())
                throw std::runtime_error("Surface format is not initialized.");
            return surfaceFormat.value().format;
        }
        [[nodiscard]] auto activeImageIndex_v() const{
            if (!activeImageIndex.has_value())
                throw std::runtime_error("Active image index is not set.");
            return activeImageIndex.value();
        }
        [[nodiscard]] const auto& renderPass_v() const{
            if (!renderPass.has_value())
                throw std::runtime_error("Render pass is not initialized.");
            return renderPass.value();
        }
        [[nodiscard]] const auto& framebuffers_v() const {
            if (!framebuffers.has_value())
                throw std::runtime_error("Framebuffers are not initialized.");
            return framebuffers.value();
        }

        [[nodiscard]] const auto& offscreenRenderPass_v(const std::string& id) const {
            if (!offscreenRenderPass.has_value())
                throw std::runtime_error("Offscreen render passes are not initialized.");

            auto& map = offscreenRenderPass.value();
            auto it = map.find(id);
            if (it == map.end())
                throw std::runtime_error("Render pass with the specified ID does not exist.");

            return it->second;
        }

        [[nodiscard]] const auto& offscreenFramebuffers_v(const std::string& id) const {
            if (!offscreenFramebuffers.has_value())
                throw std::runtime_error("Offscreen framebuffers are not initialized.");

            auto& map = offscreenFramebuffers.value();
            auto it = map.find(id);
            if (it == map.end())
                throw std::runtime_error("Framebuffer with the specified ID does not exist.");

            return it->second;
        }

    };


    struct vkCommandBufContext{
        struct_y(vkCommandBufContext,
                 (opt<vk::CommandBuffer>, cmd));
    };

}

#endif //VKCELSHADINGRENDERER_VKEVENTTYPES_H
