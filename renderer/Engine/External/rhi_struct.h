//
// Created by lenovo on 9/25/2024.
//

#ifndef VKCELSHADINGRENDERER_RHI_STRUCT_H
#define VKCELSHADINGRENDERER_RHI_STRUCT_H

#include "stl_mimalloc.h"
#include "spdlog.h"

namespace vot {

    template<typename R, typename T>
    R to_value(T t) {
        return static_cast<R>(t);
    }

    template<typename T>
    uint32_t to_u32(T t) {
        return static_cast<uint32_t>(t);
    }

    template<typename T>
    uint64_t to_u64(T t) {
        return static_cast<uint64_t>(t);
    }

}

namespace vot::inline ui{
    enum uiWidget : int;
}

namespace vot::inline rhi{

    enum class queueType{ eGraphics, eTransfer, eCount, eUndefined};

    enum threadSpecificCmdPool{
        eMainRender,
        eSceMain,
        eCount, eUndefined
    };

    enum class timelineStage : uint64_t{
        ePrepare = 0,     ///
        eRender = 100,    ///
        eFinish = 200,    ///
        ePresent = 201,   ///
    };

    enum memoryUsage{
        eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
        eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
        eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
        eGpuToCPU = VMA_MEMORY_USAGE_GPU_TO_CPU,
    };

    enum allocStrategy{
        eDefault = 0,
        eMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
        eMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        eHostSequentialWrite = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        eDedicated = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
    };

    enum imageFlagBits : uint32_t{
        eNone = 0,
        eDepth = 1 << 0,
        eDepthStencil = 1 << 1,
        eDynamicRender = 1 << 2,
//        eUpdateColorToImGui = 1 << 3,
//        eUpdateDepthToImGui = 1 << 4,
    };

    enum RTShaderRole{
       eGeneral,
       eClosestHit,
       eAnyHit,
       eIntersection
    };

    using imageFlags = std::underlying_type_t<imageFlagBits>;

    inline imageFlags operator|(imageFlagBits lhs, imageFlagBits rhs) {
        return static_cast<imageFlagBits>(
                static_cast<std::underlying_type_t<imageFlagBits>>(lhs) |
                static_cast<std::underlying_type_t<imageFlagBits>>(rhs)
        );
    }

    struct Resolutions {
        static constexpr inline vk::Extent2D eHDExtent{1280, 720};
        static constexpr inline vk::Extent2D eFullHDExtent{1920, 1080};
        static constexpr inline vk::Extent2D eQHDExtent{2560, 1440};
        static constexpr inline vk::Extent2D e4UHDExtent{3840, 2160};
        static constexpr inline vk::Extent2D e8UHDExtent{7680, 4320};


        static auto viewport(const vk::Extent2D& extent) -> vk::Viewport{
            return {0.f, 0.f, (float)extent.width, (float)extent.height, 0.f, 1.f};
        }
        static auto scissor(const vk::Extent2D& extent) -> vk::Rect2D{
            return {{0, 0}, extent};
        }
    };

    struct FrameEntry {
        vk::Image image;
        vk::ImageView imageView;
        vk::Semaphore readSemaphore;
        vk::Semaphore writtenSemaphore;
    };


struct ImageDrawCI{
    SS(vk::ImageLayout, oldLayout, OldLayout);
    SS(vk::ImageLayout, newLayout, NewLayout);
    SS(vk::AccessFlags2KHR, srcAccessMask, SrcAccessMask);
    SS(vk::AccessFlags2KHR, dstAccessMask, DstAccessMask);
    SS(vk::PipelineStageFlags2KHR, srcStageMask, SrcStageMask);
    SS(vk::PipelineStageFlags2KHR, dstStageMask, DstStageMask);
    SS(vk::ImageSubresourceRange, subresourceRange, SubresourceRange);
};

struct DescriptorHandle{
    uint32_t setCount{};
    uint32_t startIndex{};
    vk::DescriptorSet* pSet = nullptr;
};

    struct IPipeline{
        virtual auto acquire() -> vk::Pipeline& = 0;
        virtual auto acquirePipelineBindPoint() -> vk::PipelineBindPoint = 0;
    };

    struct CommandBuffer : public vk::CommandBuffer {
        vk::Fence fence;

        using cVk = vk::CommandBuffer;
        auto& bindPipeline_(auto& pipeline){
            cVk ::bindPipeline(pipeline.acquirePipelineBindPoint(), pipeline.acquire());
            return *this;
        }
        auto& bindPipeline_(IPipeline& pipeline){
            cVk ::bindPipeline(pipeline.acquirePipelineBindPoint(), pipeline.acquire());
            return *this;
        }
        auto& setRenderArea_(const vk::Extent2D& extent){
            cVk ::setViewport(0, Resolutions::viewport(extent));
            cVk ::setScissor(0, Resolutions::scissor(extent));
            return *this;
        }

        auto& bindVertexBuffers(auto& buffer){
            cVk ::bindVertexBuffers(0, buffer->buffer, {0});
            return *this;
        }

        auto& bindIndexBuffer_(auto& buffer){
            cVk ::bindIndexBuffer(buffer->buffer, 0, vk::IndexType::eUint32);
            return *this;
        }

        auto& bindDescriptorSets_(auto& pipeline, const DescriptorHandle& set, const uint32_t& index = 0){
            cVk ::bindDescriptorSets(pipeline.acquirePipelineBindPoint(), pipeline.acquirePipelineLayout(),
                                     set.startIndex, set.pSet[index], nullptr);

            return *this;
        }

        auto& bindDescriptorSets_(auto& pipeline, const uint32_t& index = 0){
            cVk ::bindDescriptorSets(pipeline.acquirePipelineBindPoint(), pipeline.acquirePipelineLayout(),
                                     pipeline.DS.startIndex, pipeline.DS.pSet[index], nullptr);

            return *this;
        }

        using cVk ::traceRaysKHR;
        auto& traceRaysKHR_(auto& pipeline, const vk::Extent2D& extent, const uint32_t& depth, vk::DispatchLoaderDynamic* dynamicDispatcher){
            cVk ::traceRaysKHR(pipeline.gRgen(), pipeline.gMiss(),
                               pipeline.gHit(), pipeline.gCall(),
                               extent.width, extent.height, depth, *dynamicDispatcher);

            return *this;
        }

        auto render(const std::function<void()>& fn, vk::CommandBufferUsageFlags flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit){
            cVk ::begin(vk::CommandBufferBeginInfo().setFlags(flags));

            fn();

            cVk ::end();
        }
    };

    using RHandle = CommandBuffer*;

//    struct DescriptorSetLayoutBinding {
//        uint32_t set{};
//        uint32_t binding{};
//        vk::DescriptorType descriptorType{};
//        uint32_t descriptorCount{};
//        vk::ShaderStageFlags shaderStageFlags{};
//        const vk::Sampler *pImmutableSamplers{};
//    };

    struct PipelineDescriptorSetLayoutCI{
        vot::map<uint32_t, vot::vector<vk::DescriptorSetLayoutBinding>> desSetBindings{};
        vot::vector<vk::DescriptorSetLayout> desSetLayouts{};
        vot::vector<vk::PushConstantRange> pushConstantRange{};
        vot::vector<vk::DescriptorPoolSize> poolSizes{};
        uint32_t maxSets{};

        auto& addDescriptorSetLayoutBinding(const uint32_t& set, const uint32_t& binding, const vk::DescriptorType& type, vk::ShaderStageFlags flags, const uint32_t& count = 1) {
            desSetBindings[set].emplace_back(binding, type, count, flags);
            poolSizes.emplace_back(type, count);
            if (maxSets < set) maxSets = set;
            return *this;
        }
        auto buildDescriptorSetLayouts(vk::Device* device) -> void{
            d = device;
            if (desSetLayouts.empty() && !desSetBindings.empty()) {
                for (auto &bds: desSetBindings) {
                    vk::DescriptorSetLayoutCreateInfo createInfo{{}, bds.second};

                    desSetLayouts.emplace_back( vot::create("create descriptor set layout") = [&] {
                        return device->createDescriptorSetLayout(createInfo);
                    });
                }
            }
        }
        auto buildPipelineSetLayout(vk::Device* device) -> vk::PipelineLayout {
            d = device;

            buildDescriptorSetLayouts(device);

            vk::PipelineLayoutCreateInfo createInfo{ {}, desSetLayouts, pushConstantRange };

            return vot::create("create pipeline layout") = [&]{
                return device->createPipelineLayout(createInfo);
            };
        }

        auto clear() -> void{
            for(auto& setLayout : desSetLayouts){
                d->destroy(setLayout);
            }
        }
    private:
        vk::Device* d;
    };

    struct PipelineDescriptorSetLayoutCI2{
        vot::vector<vk::DescriptorSetLayoutBinding> globalSetLayoutBinding = []{
            return vot::vector<vk::DescriptorSetLayoutBinding>{
                    vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}
            };
        }();

    public:
        vot::map<uint32_t, vot::vector<vk::DescriptorSetLayoutBinding>> setLayoutBindings{};
        vot::vector<vk::PushConstantRange> pushConstantRange{};
        vot::vector<vk::DescriptorSetLayout> desSetLayouts{};

        auto& addDescriptorSetLayoutBinding(const uint32_t& setIndex, const uint32_t& binding, const vk::DescriptorType& type, vk::ShaderStageFlags flags, const uint32_t& count = 1){
            setLayoutBindings[setIndex].emplace_back(binding, type, count, flags);
            return *this;
        }

        auto& addPushConstantRange(const vk::PushConstantRange& range){
            pushConstantRange.emplace_back(range);
            return *this;
        }

        auto& buildDescriptorSetLayouts(vk::Device* device) {
            auto buildDesSetLayout = [&](vot::vector<vk::DescriptorSetLayoutBinding>& bindings){
                vk::DescriptorSetLayoutCreateInfo ci{{}, bindings};
                return vot::create("create descriptor set layout") = [&]{
                    return device->createDescriptorSetLayout(ci);
                };
            };
            if (desSetLayouts.empty()){
                //desSetLayouts.emplace_back(buildDesSetLayout(globalSetLayoutBinding));
                for(auto& [i, bds] : setLayoutBindings){
                    desSetLayouts.emplace_back(buildDesSetLayout(bds));
                }
            }
            return *this;
        }

        auto buildPipelineSetLayout(vk::Device* device) -> vk::PipelineLayout{
            buildDescriptorSetLayouts(device);
            vk::PipelineLayoutCreateInfo createInfo{{}, desSetLayouts, pushConstantRange};

            return vot::create("create pipeline laout") = [&]{
                return device->createPipelineLayout(createInfo);
            };
        }

        auto clear(vk::Device* device) -> void{
            for(auto& setLayout : desSetLayouts){
                device->destroy(setLayout);
            }
        }
    };

    struct VertexInputInterfaceCI{
        //std::initializer_list<vk::VertexInputBindingDescription> vertexInputBindings;
        vot::vector<vk::VertexInputBindingDescription> vertexInputBindings;
        //std::initializer_list<vk::VertexInputAttributeDescription> vertexInputAttributes;
        vot::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
        //std::optional<vk::PrimitiveTopology> primitiveTopology;
        std::optional<vk::PipelineCreateFlags> pipelineCreateFlags;

        SS_OPT(vk::PrimitiveTopology, primitiveTopology, PrimitiveTopology);
        auto& addVertexInputBindingDescription(const uint32_t& binding, const uint32_t& stride, const vk::VertexInputRate& inputRate){ vertexInputBindings.emplace_back(binding, stride, inputRate); return *this; }
        auto& addVertexInputAttributeDescription(const uint32_t& location, const uint32_t& binding, const vk::Format& format, const uint32_t& offset) { vertexInputAttributes.emplace_back(location, binding, format, offset); return *this; }
    } ;
    struct PreRasterizationShadersCI{
        vot::string shaderPt;
        vot::string geomShaderPt;
        std::optional<vk::PipelineCreateFlags> pipelineCreateFlags;
        std::initializer_list<vk::DynamicState> dynamicStates;
        std::initializer_list<vk::Viewport> viewports;
        std::initializer_list<vk::Rect2D> rect2d;

        SS_OPT(uint32_t, depthBiasEnable, DepthBiasEnable);
        SS_OPT(uint32_t, depthBiasConstantFactor, DepthBiasConstantFactor);
        SS_OPT(uint32_t, depthBiasSlopeFactor, DepthBiasSlopeFactor);
        auto& setShaderPath(vot::string pt) { shaderPt = std::move(pt); return *this; }
        auto& setGeomShaderPath(vot::string pt) { geomShaderPt = std::move(pt); return *this; }
    };
    struct FragmentOutputInterfaceCI{
        std::initializer_list<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    };
    struct FragmentShaderCI{
        vot::string shaderPt;

        SS_OPT(vk::Bool32, depthTestEnable, DepthTestEnable);
        SS_OPT(vk::Bool32, depthWriteEnable, DepthWriteEnable);
        auto& setShaderPath(vot::string pt) { shaderPt = std::move(pt); return *this; }
    };
    struct PipelineRenderingCI{
        mutable vot::vector<vk::Format> colorAttachmentFormats_dynamicRenderingEx;
        vk::Format depthAttachmentFormat_dynamicRenderingEx = vk::Format::eD32SfloatS8Uint;
        vk::Format stencilAttachmentFormat_dynamicRenderingEx = vk::Format::eD32SfloatS8Uint;

        auto& setColorAttachmentFormats(const vk::Format& format) { colorAttachmentFormats_dynamicRenderingEx.emplace_back(format); return *this; }

        auto acquire(const bool& useDepthFormat) -> vk::PipelineRenderingCreateInfoKHR{
            auto createInfo = vk::PipelineRenderingCreateInfoKHR()
                    .setColorAttachmentFormats(colorAttachmentFormats_dynamicRenderingEx);

            if (useDepthFormat){
                createInfo.setDepthAttachmentFormat(depthAttachmentFormat_dynamicRenderingEx)
                        .setStencilAttachmentFormat(stencilAttachmentFormat_dynamicRenderingEx);
            }

            return createInfo;
        }
    };
    struct RenderPass2CI{
        vot::vector<vk::AttachmentDescription2> attachmentDescription2s;
        vot::vector<vk::SubpassDependency2> subpassDependency2s;
        vot::vector<vk::SubpassDescription2> subpassDescription2s;
    //    PipelineRenderingCI pipelineRenderingCi;
        bool useRenderingDepth{false};

        vot::vector<vk::Format> colorAttachmentFormats_dynamicRenderingEx;
        vk::Format depthAttachmentFormat_dynamicRenderingEx = vk::Format::eD32SfloatS8Uint;
        vk::Format stencilAttachmentFormat_dynamicRenderingEx = vk::Format::eD32SfloatS8Uint;

        auto& setColorAttachmentFormats(vk::Format format) { colorAttachmentFormats_dynamicRenderingEx.push_back(format); return *this; }


        auto& setAttachmentDescription2(const vot::vector<vk::AttachmentDescription2>& attachDesc2s) { attachmentDescription2s = attachDesc2s; return *this; }
        auto& setSubpassDependency2(const vot::vector<vk::SubpassDependency2>& subpassDep2s) { subpassDependency2s = subpassDep2s; return *this; }
        auto& setSubpassDescription2(const vot::vector<vk::SubpassDescription2>& subpassDesc2s) { subpassDescription2s = subpassDesc2s; return *this; }
    //    auto& setPipelineRenderingCreateInfo(const vk::PipelineRenderingCreateInfo& renderingCI) { pipelineRenderingCreateInfo = renderingCI; return *this; }
        auto& enableRenderingDepth() { useRenderingDepth = true; return *this; }
        auto& setRenderingDepth(const vk::Bool32& enableRenderingDepth = vk::False) { useRenderingDepth = enableRenderingDepth; return *this; }

        auto getPipelineRenderingCreateInfo() -> vk::PipelineRenderingCreateInfoKHR{
            auto createInfo = vk::PipelineRenderingCreateInfoKHR()
                    .setColorAttachmentFormats(colorAttachmentFormats_dynamicRenderingEx);

            if (useRenderingDepth){
                createInfo.setDepthAttachmentFormat(depthAttachmentFormat_dynamicRenderingEx)
                        .setStencilAttachmentFormat(stencilAttachmentFormat_dynamicRenderingEx);
            }

            return createInfo;
        }

        auto getInheritanceRenderingInfo() -> vk::CommandBufferInheritanceRenderingInfoKHR {
            auto info = vk::CommandBufferInheritanceRenderingInfo()
                    .setColorAttachmentFormats(colorAttachmentFormats_dynamicRenderingEx);

            if (useRenderingDepth){
                info.setDepthAttachmentFormat(depthAttachmentFormat_dynamicRenderingEx)
                    .setStencilAttachmentFormat(stencilAttachmentFormat_dynamicRenderingEx);
            }
            return info;
        }
    };
    struct PipelineLibrary{
        vk::RenderPass renderPass{};
        vk::PipelineLayout pipelineLayout{};
        vk::Pipeline vertexInputInterface{};
        vk::Pipeline preRasterizationShaders{};
        vk::Pipeline fragmentOutputInterface{};
        vk::Pipeline fragmentShader{};

        RenderPass2CI renderPass2CI;
//        PipelineDescriptorSetLayoutCI pipelineDescriptorSetLayoutCI;
//        PipelineDescriptorSetLayoutCI2 pipelineDescriptorSetLayoutCI2;
        std::variant<PipelineDescriptorSetLayoutCI, PipelineDescriptorSetLayoutCI2> pipelineDescriptorSetLayoutCI;
        VertexInputInterfaceCI vertexInputInterfaceCI;
        PreRasterizationShadersCI preRasterizationShadersCI;
        FragmentOutputInterfaceCI fragmentOutputInterfaceCI;
        FragmentShaderCI fragmentShaderCI;

        auto& setVertexInputInterfaceCI(const struct VertexInputInterfaceCI& ci) { vertexInputInterface = VK_NULL_HANDLE; vertexInputInterfaceCI = ci; return *this; };
        auto& setPreRasterizationShadersCI(const struct PreRasterizationShadersCI& ci) { preRasterizationShaders = VK_NULL_HANDLE; preRasterizationShadersCI = ci; return *this; };
        auto& setFragmentOutputInterfaceCI(const struct FragmentOutputInterfaceCI& ci) { fragmentOutputInterface = VK_NULL_HANDLE; fragmentOutputInterfaceCI = ci; return *this; };
        auto& setFragmentShaderCI(const struct FragmentShaderCI& ci) { fragmentShader = VK_NULL_HANDLE; fragmentShaderCI = ci; return *this; };
        auto& setPipelineDescriptorSetLayoutCI(const struct PipelineDescriptorSetLayoutCI& ci) { refreshPipelineStages(); pipelineLayout = VK_NULL_HANDLE; pipelineDescriptorSetLayoutCI = ci; return *this; };
        auto& setPipelineDescriptorSetLayoutCI2(const struct PipelineDescriptorSetLayoutCI2& ci2) { refreshPipelineStages(); pipelineLayout = VK_NULL_HANDLE; pipelineDescriptorSetLayoutCI = ci2; return *this; };
//        auto& setPipelineDescriptorSetLayoutCI(const struct PipelineDescriptorSetLayoutCI& ci) { refreshPipelineStages(); pipelineLayout = VK_NULL_HANDLE; pipelineDescriptorSetLayoutCI = ci; return *this; };
//        auto& setPipelineDescriptorSetLayoutCI2(const struct PipelineDescriptorSetLayoutCI2& ci2) { refreshPipelineStages(); pipelineLayout = VK_NULL_HANDLE; pipelineDescriptorSetLayoutCI2 = ci2; return *this; };
        auto& setRenderPass2CI(const struct RenderPass2CI& ci) { refreshPipelineStages(); renderPass = VK_NULL_HANDLE; renderPass2CI = ci; return *this; }
//        auto useDynamicRendering() { renderPass = VK_NULL_HANDLE; return *this; }
    private:
        auto refreshPipelineStages() -> void { preRasterizationShaders = VK_NULL_HANDLE; fragmentOutputInterface = VK_NULL_HANDLE; fragmentShader = VK_NULL_HANDLE; }
    };

    struct BufferCI{

    };

    struct ImageCI{
        //imageFlags imageFlags = eColor;
        imageFlags imageFlags = eDefault;
        uint8_t imageCount = 1;
        vk::ImageType imageType = vk::ImageType::e2D;
        vk::Format format = vk::Format::eR8G8B8A8Unorm;
        vk::Extent3D extent = {2560, 1440, 1};
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        vk::SampleCountFlagBits sampleCountFlags = vk::SampleCountFlagBits::e1;
        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive;

        vk::ImageViewType imageViewType = vk::ImageViewType::e2D;
        vk::ComponentSwizzle componentSwizzle = vk::ComponentSwizzle::eIdentity;
        vk::ImageSubresourceRange imageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

        vk::Filter magFilter = vk::Filter::eLinear;
        vk::Filter minFilter = vk::Filter::eNearest;
        vk::SamplerMipmapMode samplerMipMap = vk::SamplerMipmapMode::eLinear;
        vk::SamplerAddressMode u = vk::SamplerAddressMode::eRepeat;
        vk::SamplerAddressMode v = vk::SamplerAddressMode::eRepeat;
        vk::SamplerAddressMode w = vk::SamplerAddressMode::eRepeat;
        vk::ImageLayout currentImageLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout currentDepthImageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        float mipLodBias = 0.f;
        vk::Bool32 anisotropyEnable = vk::False;
        float maxAnisotropy = 1.f;
        vk::Bool32 compareEnable = vk::False;
        vk::CompareOp compareOp = vk::CompareOp::eAlways;
        float minLod = 0.f;
        float maxLod = 0.f;
        vk::BorderColor borderColor = vk::BorderColor::eIntOpaqueBlack;
        vk::Bool32  unNormalizedCoordinates = vk::False;
        vk::RenderPass renderPass;
        std::optional<vot::uiWidget> uiWidget = std::nullopt;
        bool custom_define = false;

        vk::ClearColorValue clearColorValue = vk::ClearColorValue{0.2f, 0.2f, 0.2f, 1.f};

        explicit ImageCI(vk::Extent2D e2d = Resolutions::eQHDExtent, uint8_t imgCount = 1) : extent(e2d, 1), imageCount(imgCount) {};
        explicit ImageCI(vk::Extent2D e2d, vot::imageFlags flags, uint8_t imgCount = 1) : extent(e2d, 1), imageFlags(flags), imageCount(imgCount) {};
        ImageCI(vk::Extent2D e2d, vot::imageFlags flags, vk::RenderPass rp, uint8_t imgCount = 1) : extent(e2d, 1), imageFlags(flags),  renderPass(rp), imageCount(imgCount) {};

        ImageCI& setRenderPass(vk::RenderPass rp){ renderPass = rp; return *this; }
        ImageCI& setFlags(vot::imageFlags flags) { imageFlags = flags; return *this; }
        ImageCI& setImageType(vk::ImageType type) { imageType = type; return *this;}
        ImageCI& setFormat(vk::Format f) { format = f; return *this; }
        ImageCI& setExtent(vk::Extent3D e) { extent = e; return *this; }
        ImageCI& setExtent(vk::Extent2D e) { extent = vk::Extent3D{e, 1}; return *this; }
        ImageCI& setMipLevels(uint32_t levels) { mipLevels = levels; return *this;}
        ImageCI& setArrayLayers(uint32_t layers) { arrayLayers = layers; return *this; }
        ImageCI& setSampleCountFlags(vk::SampleCountFlagBits flags) { sampleCountFlags = flags; return *this; }
        ImageCI& setTiling(vk::ImageTiling t) { tiling = t; return *this; }
        ImageCI& addUsage(vk::ImageUsageFlags usg) { usage |= usg; return *this; }
        ImageCI& setUsage(vk::ImageUsageFlags usg) { usage = usg; return *this; }
        ImageCI& setSharingMode(vk::SharingMode mode) { sharingMode = mode; return *this; }
        ImageCI& setAspect(vk::ImageAspectFlags flags){ imageSubresourceRange.aspectMask = flags; return *this;}
        ImageCI& setImageCount(uint8_t count){ imageCount = count; return *this; }
        ImageCI& setDstImageLayout(vk::ImageLayout imageLayout){ currentImageLayout = imageLayout; return *this; }
        ImageCI& setDstDepthImageLayout(vk::ImageLayout imageLayout){ currentDepthImageLayout = imageLayout; return *this;}
        ImageCI& updateColorToImGui(vot::uiWidget widget) { uiWidget = widget; return *this; }

        template<typename T>
        ImageCI& setExtent(T w_, T h_) { extent = vk::Extent3D{static_cast<uint32_t>(w_), static_cast<uint32_t>(h_), 1}; return *this; }
    };

    struct SubmitInfo{
        vot::vector<uint64_t> waitValues{};
        vot::vector<uint64_t> signalValues{};
        vot::vector<vot::CommandBuffer> cmds{};
        vot::vector<vk::PipelineStageFlags> waitStageMasks{};
        vk::Fence fence{};
        vk::Semaphore waitSemaphore{};
        vk::Semaphore signalSemaphore{};
        queueType queue{};
        uint32_t selectQueue{};
        inline static uint64_t counter = 0;
        bool onetimeSubmit{false};
        void* pNext;

        auto& setWaitValues(timelineStage stage){ auto v = to_u64(stage) + counter; waitValues.emplace_back(v); return *this; }
        auto& setWaitValues(const vot::vector<timelineStage>& stages){ for(const auto& s : stages){ waitValues.emplace_back(to_u64(s) + counter); } return *this; }
        auto& setSignalValues(timelineStage stage){ auto v = to_u64(stage) + counter; signalValues.emplace_back(v); return *this;}
        auto& setSignalValues(const vot::vector<timelineStage>& stages){ for(const auto& s : stages){ signalValues.emplace_back(to_u64(s) + counter); } return *this; }
        auto& setWaitSemaphore(const vk::Semaphore& wait){ waitSemaphore = wait; return *this; }
        auto& setSignalSemaphore(const vk::Semaphore& signal){ signalSemaphore = signal; return *this; }
        auto& setWaitStageMasks(const vot::vector<vk::PipelineStageFlags>& waitMasks){ this->waitStageMasks = waitMasks; return *this; }
        auto& setWaitStageMasks(const vk::PipelineStageFlags& waitStageMask){ this->waitStageMasks.emplace_back(waitStageMask); return *this; }
        auto& setFence(const vk::Fence& f){ fence = f; return *this; }
        auto& setCommandBuffers(const vot::CommandBuffer& c){ cmds.emplace_back(c); return *this; }
        auto& setCommandBuffers(const vot::vector<vot::CommandBuffer>& c){ cmds = c; return *this; }
        auto& setQueueType(const queueType& type){ queue = type; return *this; }
        auto& setSelectQueue(const uint32_t& select){ selectQueue = select; return *this; }
        auto& useOnetimeSubmit() { onetimeSubmit = true; return *this; }

        auto& setRHandle(const RHandle& handle) { pNext = handle; return *this; }
        static auto increase(){ counter += to_u64(vot::timelineStage::ePresent); }
    };


    struct DescriptorPoolSize{
        vk::DescriptorType type;
        float countOrRatio;

        [[nodiscard]] vk::DescriptorPoolSize toVk(uint32_t maxSets) const {
            auto count = 0u;
            if (countOrRatio >= 1.f){
                count = static_cast<uint32_t>(std::floor(countOrRatio));
            } else {
                auto x = static_cast<uint32_t>(std::floor((float)maxSets * countOrRatio));
                count = std::max<uint32_t>(1, x);
            }
            return {type, count};
        }
    };

    struct DescriptorPoolCreateInfo{
        DescriptorPoolCreateInfo() = default;
        DescriptorPoolCreateInfo(DescriptorPoolCreateInfo&& other) noexcept
                : mMaxSets(other.mMaxSets),
                  sizes(std::move(other.sizes)),
                  ci(other.ci)
        {
            ci.setPoolSizes(sizes);
        }

        DescriptorPoolCreateInfo(const DescriptorPoolCreateInfo& other)
                : mMaxSets(other.mMaxSets),
                  sizes(other.sizes),
                  ci(other.ci)
        {
            ci.setPoolSizes(sizes);
        }

        auto setFlags(const vk::DescriptorPoolCreateFlags& flags){
            ci.setFlags(flags);
            return *this;
        }

        auto setMaxSets(const uint32_t& maxSets){
            mMaxSets = maxSets;
            ci.setMaxSets(maxSets);
            return *this;
        }

        auto setPoolSizes(const vot::vector<DescriptorPoolSize>& poolSizes){
            sizes.clear();
            sizes.reserve(poolSizes.size());
            for(auto const& ps : poolSizes){
                sizes.push_back(ps.toVk(mMaxSets));
            }
            ci.setPoolSizes(sizes);
            return *this;
        }

        operator vk::DescriptorPoolCreateInfo() const{
            return ci;
        }

        vk::DescriptorPoolCreateInfo ci;
    private:
        uint32_t mMaxSets{};
        vot::vector<vk::DescriptorPoolSize> sizes;
    };

struct DescriptorLayout2{
    using descriptorInfo = std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR>;
    using descVec1D = vot::vector<descriptorInfo>;
    using descVec2D = vot::vector<descVec1D>;
    using descVec3D = vot::vector<descVec2D>;
    using _1d = vot::vector<descriptorInfo>;
    using _2d = vot::vector<descVec1D>;
    using _3d = vot::vector<descVec2D>;

    auto emplace(const descVec3D& layout){
        infos = layout;
        return *this;
    }
    auto emplace(const descVec2D& layout){
        infos.emplace_back(layout);
        return *this;
    }
    auto emplace(const descVec1D& bindings){
        infos.emplace_back(descVec2D{bindings});
        return *this;
    }

    DescriptorLayout2() = default;
    explicit DescriptorLayout2(_3d  infos_) : infos(std::move(infos_)) {}
    explicit DescriptorLayout2(const _2d& layout) { infos.emplace_back(layout); }
    explicit DescriptorLayout2(const _1d& layout) { infos.emplace_back(_2d {layout}); }
    explicit DescriptorLayout2(const descriptorInfo& info) { infos.emplace_back(_2d { _1d {info}}) ;}

    operator vot::vector<vot::vector<vot::vector<descriptorInfo>>>(){
        return std::move(infos);
    }
private:
    vot::vector<vot::vector<vot::vector<descriptorInfo>>> infos;
};

}

#endif //VKCELSHADINGRENDERER_RHI_STRUCT_H
