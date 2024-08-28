//
// Created by lenovo on 4/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKCOMMON_H
#define VKCELSHADINGRENDERER_VKCOMMON_H

#include "Engine/Utils/Log.h"

enum class QueueType {
    eGraphics, eTransfer,
};

struct PipelineDesSetLayout{
    vk::PipelineLayout pipelineLayout{};
    std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings{};
    std::vector<vk::DescriptorSetLayout> desSetLayouts{};
    std::vector<vk::PushConstantRange> pushConstantRange{};
    std::vector<vk::DescriptorPoolSize> poolSize{};
    uint32_t maxSets{};

    explicit PipelineDesSetLayout(vk::Device device) : mDevice(device) {}
    ~PipelineDesSetLayout(){ mDevice.destroy(pipelineLayout);
        for(auto& layout : desSetLayouts){
            mDevice.destroy(layout);
        }
    }

    void addDesSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &bds) {
        for(const auto& bind : bds){
            poolSize.emplace_back(bind.descriptorType, bind.descriptorCount);
        }

        maxSets++;

        bindings.push_back(bds);
        vk::DescriptorSetLayoutCreateInfo createInfo{{}, bds};

        desSetLayouts.emplace_back(
                vkCreate("create descriptor set layout") = [&] {
                    return mDevice.createDescriptorSetLayout(createInfo);
                });
    }

     void addDesSetLayout(const uint32_t &set, const uint32_t &binding,
                                                const vk::DescriptorType &descriptorType,
                                                const uint32_t &descriptorCount, const vk::ShaderStageFlags &flags) {
        poolSize.emplace_back(descriptorType, descriptorCount);

        if ((set + 1) > bindings.size())
            bindings.resize(set + 1);

        bindings[set].emplace_back(binding, descriptorType, descriptorCount, flags);

         maxSets = set;
    }

    PipelineDesSetLayout& addPushConstantRange(const vk::ShaderStageFlags& flags, uint32_t offset, uint32_t size){
        pushConstantRange.emplace_back(flags, offset, size);
        return *this;
    }

    PipelineDesSetLayout& createDesSetLayout(){
        if (desSetLayouts.empty() && !bindings.empty()) {
            for (auto &bds: bindings) {
                vk::DescriptorSetLayoutCreateInfo createInfo{{}, bds};

                desSetLayouts.emplace_back(
                        vkCreate("create descriptor set layout") = [&] {
                            return mDevice.createDescriptorSetLayout(createInfo);
                        });
            }
        }

        return *this;
    }

    [[nodiscard]] inline auto getPipelineLayout() {
        if (desSetLayouts.empty() && !bindings.empty()) {
            for (auto &bds: bindings) {
                vk::DescriptorSetLayoutCreateInfo createInfo{{}, bds};

                desSetLayouts.emplace_back(
                        vkCreate("create descriptor set layout") = [&] {
                            return mDevice.createDescriptorSetLayout(createInfo);
                        });
            }
        }

        vk::PipelineLayoutCreateInfo createInfo{
                {}, desSetLayouts, pushConstantRange
        };

        return pipelineLayout ? pipelineLayout : pipelineLayout = vkCreate("create pipeline layout") = [&] {
            return mDevice.createPipelineLayout(createInfo);
        };
    }
private:
    vk::Device mDevice;
};

namespace fn{

    inline auto addRequiredExtensions(){
        uint32_t count{};
        auto glfwExtensions{glfwGetRequiredInstanceExtensions(&count)};

        return std::vector<const char*>(glfwExtensions, glfwExtensions + count);
    }

    template<typename vkProperties, typename NameFunc>
    inline bool checkSupport(const vkProperties& properties, std::vector<const char*> requiredExs, NameFunc name, bool print = false){
        std::vector<const char*> foundExs;
        for(auto& pro : properties){
            auto it = std::find_if(requiredExs.begin(), requiredExs.end(), [&](const char* requiredEx){
                return strcmp(name(pro), requiredEx) == 0;
            });
            if (it != requiredExs.end()){
                if_debug{ vkWarn("Enable extension or layer : {0}", name(pro));}
                foundExs.emplace_back(*it);
            } else{
                if_debug{ if (print) { vkTrance(name(pro));}}
            }
        }

        for(auto& found : foundExs){
            requiredExs.erase(std::remove(requiredExs.begin(), requiredExs.end(), found), requiredExs.end());
        }

        if (!requiredExs.empty()){
            if_debug{ vkInfo("the following required items were not found: "); }
            for (const auto& missingEx : requiredExs) {
                if_debug{ vkError(missingEx); }
            }
            return false;
        }

        return true;
    }

    inline bool checkInstanceSupport(const std::vector<const char*>& extensions, const std::vector<const char*>& layers, bool print = false){
        auto extensionsSupport = checkSupport(vk::enumerateInstanceExtensionProperties(), extensions,
                     [](const auto& ex){ return ex.extensionName; }, print);
        auto layersSupport = checkSupport(vk::enumerateInstanceLayerProperties(), layers,
                     [](const auto& lay){ return lay.layerName; });

        return extensionsSupport && layersSupport;
    }

    inline bool checkPhysicalSupport(const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions, bool print = false){
        auto extensionSupport = checkSupport(physicalDevice.enumerateDeviceExtensionProperties(), extensions,
                                             [](const auto& ex){ return ex.extensionName;}, print);

        return extensionSupport;
    }

    inline std::optional<uint32_t> findQueueFamily(vk::PhysicalDevice physicalDevice, QueueType type, bool print = false){
        auto families = physicalDevice.getQueueFamilyProperties();
        if (print) {
            if_debug { vkTrance("physical device support {0} queue families! ", families.size()); }
            for (const auto &queueFamily: families) {
                std::cout << "\tQueue Flags: ";

                if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                    std::cout << "Graphics ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                    std::cout << "Compute ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
                    std::cout << "Transfer ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eSparseBinding) {
                    std::cout << "Sparse Binding ";
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eProtected) {
                    std::cout << "Protected ";
                }

                std::cout << "\n";
            }
        }

        for(size_t i = 0; const auto& f : families){
            if (type == QueueType::eGraphics){
                if (f.queueFlags & vk::QueueFlagBits::eGraphics)
                    return i;
            }

            if (type == QueueType::eTransfer){
                if ((f.queueFlags & vk::QueueFlagBits::eTransfer) &&
                    !(f.queueFlags & vk::QueueFlagBits::eGraphics) &&
                    !(f.queueFlags & vk::QueueFlagBits::eCompute)){
                    return i;
                }
            }

            i++;
        }

        return std::nullopt;
    }
};

namespace yic{
    struct vkImageConfig{
        enum ImageFlags {
            eNone = 0,
            eColor = 1 << 0,
            eDepth = 1 << 1,
            eDepthStencil = 1 << 2,
            eStorage = 1 << 3,
        };
        ImageFlags imageFlags = eColor;
        vk::ImageType imageType = vk::ImageType::e2D;
        vk::Format format = vk::Format::eR8G8B8A8Unorm;
        vk::Extent3D extent = {};
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

        //bool bFramebuffers = false;
        bool custom_define = false;

        explicit vkImageConfig(vk::Extent2D e2d){ extent = vk::Extent3D{e2d, 1}; };

        template<typename T>
        explicit vkImageConfig(T width, T height) {
            extent = vk::Extent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        };

//        vkImageConfig& enableFramebuffers(){
//            bFramebuffers = true;
//            return *this;
//        }
        vkImageConfig& setRenderPass(vk::RenderPass rp){
            renderPass = rp;
            return *this;
        }

        vkImageConfig& setImageFlags(ImageFlags flags) {
            imageFlags = flags;
            return *this;
        }

        vkImageConfig& setImageType(vk::ImageType type) {
            imageType = type;
            return *this;
        }

        vkImageConfig& setFormat(vk::Format f) {
            format = f;
            return *this;
        }

        vkImageConfig& setExtent(vk::Extent3D e) {
            extent = e;
            return *this;
        }

        template<typename T>
        vkImageConfig& setExtent(T w_, T h_) {
            extent = vk::Extent3D{static_cast<uint32_t>(w_), static_cast<uint32_t>(h_), 1};
            return *this;
        }

        vkImageConfig& setMipLevels(uint32_t levels) {
            mipLevels = levels;
            return *this;
        }

        vkImageConfig& setArrayLayers(uint32_t layers) {
            arrayLayers = layers;
            return *this;
        }

        vkImageConfig& setSampleCountFlags(vk::SampleCountFlagBits flags) {
            sampleCountFlags = flags;
            return *this;
        }

        vkImageConfig& setTiling(vk::ImageTiling t) {
            tiling = t;
            return *this;
        }

        vkImageConfig& addUsage(vk::ImageUsageFlags usg) {
            usage |= usg;
            return *this;
        }
        vkImageConfig& setUsage(vk::ImageUsageFlags usg) {
            usage = usg;
            return *this;
        }

        vkImageConfig& setSharingMode(vk::SharingMode mode) {
            sharingMode = mode;
            return *this;
        }

        vkImageConfig& setAspect(vk::ImageAspectFlags flags){
            imageSubresourceRange.aspectMask = flags;
            return *this;
        }
    };
}



#endif //VKCELSHADINGRENDERER_VKCOMMON_H











