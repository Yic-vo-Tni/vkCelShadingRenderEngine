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

    struct vkInitContext {
        struct_y(vkInitContext,
                 (opt<vk::Instance>, instance),
                 (opt<vk::DispatchLoaderDynamic>, dynamicDispatcher),
                 (opt<vk::DebugUtilsMessengerEXT>, debugMessenger));
    };

    struct vkDeviceContext {
        struct_y(vkDeviceContext,
                 (opt<vk::PhysicalDevice>, physicalDevice),
                 (opt<vk::Device>, device),
                 (opt<yic::QueueFamily>, queueFamily));
    };

//    struct vkSwapchainContext {
//        struct_y(vkSwapchainContext,
//                 (opt<vk::SwapchainKHR>, swapchain),
//                 (opt<std::vector<FrameEntry>>, frameEntries),
//                 (opt<vk::SurfaceFormatKHR>, surfaceFormat),
//                 (opt<uint32_t>, activeImageIndex));
//        struct id{
//            static constexpr const char* imguiRender{"imguiRender"};
//        };
//    };

//    struct vkFrameRenderContext {
//        struct_y(vkFrameRenderContext,
//                 (opt<vk::RenderPass>, renderPass),
//                 (opt<std::vector<vk::Framebuffer>>, framebuffers));
//
//        struct id{
//            static constexpr const char* imguiRender{"imguiRender"};
//        };
//    };

    struct vkCommandBufContext{
        struct_y(vkCommandBufContext,
                 (opt<vk::CommandBuffer>, cmd));
    };


    struct vkRenderContext {
        struct_y(vkRenderContext,
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

        const auto& swapchain_v() {
            if (!swapchain.has_value())
                throw std::runtime_error("Swapchain is not initialized.");
            return swapchain.value();
        }
        const auto& frameEntries_v(){
            if (!frameEntries.has_value())
                throw std::runtime_error("Frame entries are not initialized.");
            return frameEntries.value();
        }
        auto imageCount_v(){
            if (!frameEntries.has_value())
                throw std::runtime_error("Frame entries are not initialized -- image count.");
            return static_cast<uint32_t>(frameEntries->size());
        }
        const auto& surfaceFormat_v(){
            if (!surfaceFormat.has_value())
                throw std::runtime_error("Surface format is not initialized.");
            return surfaceFormat.value().format;
        }
        auto activeImageIndex_v(){
            if (!activeImageIndex.has_value())
                throw std::runtime_error("Active image index is not set.");
            return activeImageIndex.value();
        }
        const auto& renderPass_v(){
            if (!renderPass.has_value())
                throw std::runtime_error("Render pass is not initialized.");
            return renderPass.value();
        }
        const auto& framebuffers_v() {
            if (!framebuffers.has_value())
                throw std::runtime_error("Framebuffers are not initialized.");
            return framebuffers.value();
        }

        const auto& offscreenRenderPass_v(const std::string& id) {
            if (!offscreenRenderPass.has_value())
                throw std::runtime_error("Offscreen render passes are not initialized.");

            auto& map = offscreenRenderPass.value();
            auto it = map.find(id);
            if (it == map.end())
                throw std::runtime_error("Render pass with the specified ID does not exist.");

            return it->second;
        }

        const auto& offscreenFramebuffers_v(const std::string& id) {
            if (!offscreenFramebuffers.has_value())
                throw std::runtime_error("Offscreen framebuffers are not initialized.");

            auto& map = offscreenFramebuffers.value();
            auto it = map.find(id);
            if (it == map.end())
                throw std::runtime_error("Framebuffer with the specified ID does not exist.");

            return it->second;
        }

    };

}

#endif //VKCELSHADINGRENDERER_VKEVENTTYPES_H
