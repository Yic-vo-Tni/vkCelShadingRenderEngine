//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKEVENTTYPES_H
#define VKCELSHADINGRENDERER_VKEVENTTYPES_H

#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/vkDescriptor.h"

namespace tbb{

    template<typename T>
    inline tbb::concurrent_unordered_map<std::string, T> make_unordered_map(std::string id, T t) {
        tbb::concurrent_unordered_map<std::string, T> temp;
        temp[id] = t;
        return  temp;
    }
}

namespace et {

    struct FrameEntry {
        vk::Image image;
        vk::ImageView imageView;
        vk::Semaphore readSemaphore;
        vk::Semaphore writtenSemaphore;
    };

    struct vkSetupContext {
        HANA_OPT(vkSetupContext,
                 (vk::Instance, instance),
                 (vk::DispatchLoaderDynamic, dynamicDispatcher),
                 (vk::DebugUtilsMessengerEXT, debugMessenger),
                 (vk::PhysicalDevice, physicalDevice),
                 (vk::Device, device),
                 (yic::QueueFamily, queueFamily));


        RETURN_REF(instance);
        RETURN_REF(dynamicDispatcher);
        RETURN_REF(debugMessenger);
        RETURN_REF(physicalDevice);
        RETURN_REF(device);

        RETURN_CUSTOM_REF(qGraphicsPrimary, queueFamily, queueFamily->getPrimaryGraphicsQueue());
        RETURN_CUSTOM_REF(qGraphicsAuxiliary, queueFamily, queueFamily->getAuxiliaryGraphicsQueue());
        RETURN_CUSTOM_REF(qTransferUpload, queueFamily, queueFamily->getUploadTransferQueue());
        RETURN_CUSTOM_REF(qTransferDownload, queueFamily, queueFamily->getDownloadTransferQueue());

        RETURN_CUSTOM_VALUE(qIndexGraphicsPrimary, queueFamily, queueFamily->getPrimaryGraphicsFamilyIndex());
        RETURN_CUSTOM_VALUE(qIndexGraphicsAuxiliary, queueFamily, queueFamily->getAuxiliaryGraphicsFamilyIndex());
        RETURN_CUSTOM_VALUE(qIndexTransferUpload, queueFamily, queueFamily->getUploadTransferFamilyIndex());
        RETURN_CUSTOM_VALUE(qIndexTransferDownload, queueFamily, queueFamily->getDownloadTransferFamilyIndex());
    };

//    struct vkPipeline {
//        HANA(vkPipeline,
//             (opt < std::unordered_map<std::string, vk::Pipeline>>, graphics),
//             (opt < std::unordered_map<std::string, vk::Pipeline>>, compute),
//             (opt < std::unordered_map<std::string, vk::Pipeline>>, raytracing),
//             (opt < vk::DescriptorSet > , descriptorSet));
//    };
//
//    struct vkRenderPass{
//        HANA(vkRenderPass,
//             (opt<vk::RenderPass>, renderPass));
//
//        struct id{
//            static constexpr const char* basicRenderPass = "BasicRenderPass";
//            static constexpr const char* depthRenderPass = "DepthRenderPass";
//            static constexpr const char* shadowRenderPass = "ShadowRenderPass";
//            static constexpr const char* postProcessRenderPass = "PostProcessRenderPass";
//        };
//    };


    struct vkRenderContext {
        HANA_OPT(vkRenderContext,
                 (ImVec2, size),
                 (vk::Extent2D, extent),
                 (GLFWwindow * , window),
                 (vk::SwapchainKHR, swapchain),
                 (vk::Extent2D, currentExtent),
                 (std::vector<FrameEntry>, frameEntries),
                 (vk::SurfaceFormatKHR, surfaceFormat),
                 (uint32_t, activeImageIndex),
                 (vk::RenderPass, renderPass),
                 (std::vector<vk::Framebuffer>, framebuffers),
                 (vk::CommandBuffer, cmd)
        );

        DEFINE_ID_CONSTS(
                mainRender
        );

        RETURN_REF(window);
        RETURN_REF(swapchain);
        RETURN_REF(frameEntries);
        RETURN_REF(surfaceFormat);
        RETURN_REF(renderPass);
        RETURN_REF(framebuffers);
        RETURN_REF(cmd);

        RETURN_VALUE(extent);
        RETURN_VALUE(currentExtent);
        RETURN_VALUE(activeImageIndex);

        RETURN_CUSTOM_VALUE(width, size, size->x);
        RETURN_CUSTOM_VALUE(height, size, size->y);
        RETURN_CUSTOM_VALUE(imageCount, frameEntries, static_cast<uint32_t>(frameEntries->size()));
    };

    struct vkResource {
        HANA_OPT(vkResource,
                 (vot::concurrent_shared_ptr_unordered_map<yic::vkBuffer>, buf),
                 (vot::concurrent_shared_ptr_unordered_map<yic::vkImage>, img),
                 (vot::concurrent_shared_ptr_unordered_map<yic::vkDescriptor>, desc)
        );

        DEFINE_ID_CONSTS_EX(
                buf,
                camera
        );

        DEFINE_ID_CONSTS_EX(
                desc,
                main
        )

        RETURN_REF(buf);
        RETURN_REF(img);
        RETURN_REF(desc);
    };


    struct test {
        HANA(test,
             (opt < int > , x));

        RETURN_REF(x);
    };



}

#endif //VKCELSHADINGRENDERER_VKEVENTTYPES_H
