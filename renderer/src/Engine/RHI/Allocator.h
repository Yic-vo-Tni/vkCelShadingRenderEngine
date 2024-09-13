//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKALLOCATOR_H
#define VKCELSHADINGRENDERER_VKALLOCATOR_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"
#include "Engine/Core/FileOperator/FileOperation.h"

#include "Engine/RHI/vkCommon.h"
#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/Command.h"

using vkBuf_sptr = std::shared_ptr<yic::vkBuffer>;
//using vkBuffer_sptr = std::shared_ptr<yic::vkBuffer>;
using vkImg_sptr = std::shared_ptr<yic::vkImage>;
//using vkImage_sptr = std::shared_ptr<yic::vkImage>;
//using vkAccel_sptr = std::shared_ptr<yic::vkAccel>;


//namespace t {
//    enum ImageUsageFlagBits {
//        eTransferSrc = 0x00000001,
//        eTransferDst = 0x00000002,
//        eSampled = 0x00000004,
//    };
//
//    using ImageUsageFlags = std::underlying_type_t<ImageUsageFlagBits>;
//
//    inline ImageUsageFlags operator|(ImageUsageFlagBits lhs, ImageUsageFlagBits rhs) {
//        return static_cast<ImageUsageFlags>(
//                static_cast<std::underlying_type_t<ImageUsageFlagBits>>(lhs) |
//                static_cast<std::underlying_type_t<ImageUsageFlagBits>>(rhs)
//        );
//    }
//}

template<typename T>
concept NoBool = (!std::is_same_v<std::decay_t<T>, bool>);

template<typename ...Args>
concept ArgsPack = (... && NoBool<Args>);

namespace yic {

    class Allocator : public std::enable_shared_from_this<Allocator>{
        using imgPath = std::variant<std::string, std::vector<std::string>>;

        template<typename T>
        static void* getBufData(const std::vector<T>& data) {
            return (void*)data.data();
        }

        template<typename T>
        static void* getBufData(const std::initializer_list<T>& data) {
            return (void*)std::data(data);
        }
    public:
        enum class MemoryUsage{
            eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
            eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
            eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
            eGpuToCPU = VMA_MEMORY_USAGE_GPU_TO_CPU,
        };

        enum class AllocStrategy{
            eDefault = 0,
            eMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
            eMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            eHostSequentialWrite = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        };
    private:
        struct bufCreateInfo{
            vk::DeviceSize deviceSize;
            vk::BufferUsageFlags usage;

            MemoryUsage memoryUsage;
            AllocStrategy allocStrategy;
        };

        struct BufferHandle{
            VkBuffer buf;
            VmaAllocation vmaAllocation;
        };

    public:
        vkGet auto make = [](){ return Singleton<Allocator>::get(); };
        Allocator();
        ~Allocator();

    public:  // new
        auto allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags, MemoryUsage usage = MemoryUsage::eCpuToGpu, const std::string& id = IdGenerator::uniqueId(), bool unmap = false) -> vkBuf_sptr ;
        auto allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags, const std::string& id, MemoryUsage usage = MemoryUsage::eCpuToGpu,  bool unmap = false) -> vkBuf_sptr ;
        auto allocBuffer(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, MemoryUsage usage = MemoryUsage::eCpuToGpu, const std::string& id = IdGenerator::uniqueId(), bool unmap = false) -> vkBuf_sptr ;
        auto allocBuffer(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const std::string& id, MemoryUsage usage = MemoryUsage::eCpuToGpu, bool unmap = false) -> vkBuf_sptr ;
        auto allocBufferStaging(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags,
                             MemoryUsage usage = MemoryUsage::eGpuOnly,
                             AllocStrategy allocStrategy = AllocStrategy::eMapped,
                             const std::string &id = IdGenerator::uniqueId()) -> vkBuf_sptr;
        auto allocBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags,
                             MemoryUsage usage = MemoryUsage::eGpuOnly,
                             AllocStrategy allocStrategy = AllocStrategy::eMapped,
                             const std::string &id = IdGenerator::uniqueId()) -> vkBuf_sptr;
        auto allocBufferStaging(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags,
                             const std::string &id, MemoryUsage usage = MemoryUsage::eGpuOnly,
                             AllocStrategy allocStrategy = AllocStrategy::eMapped) -> vkBuf_sptr;
        auto allocBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags,
                             const std::string &id, MemoryUsage usage = MemoryUsage::eGpuOnly,
                             AllocStrategy allocStrategy = AllocStrategy::eMapped) -> vkBuf_sptr;

        auto allocImage(const imgPath& imgPath, std::optional<yic::ImageConfig> config = yic::ImageConfig{0}, std::string id = {}) -> vkImg_sptr;
        auto allocImage(yic::ImageConfig config = yic::ImageConfig{}, const std::string& id = IdGenerator::uniqueId()) -> vkImg_sptr;
        auto allocImage(yic2::ImageConfig config = yic2::ImageConfig{}, const std::string& id = IdGenerator::uniqueId()) -> yic2::Image_sptr;

        auto allocAccel(vk::AccelerationStructureCreateInfoKHR& createInfo) -> vkAccel_sptr;
        auto allocAccel(vk::AccelerationStructureBuildSizesInfoKHR buildSize, vk::AccelerationStructureTypeKHR type) -> vkAccel_sptr ;

/// AUTO
    private:
        struct AllocAuto {
            enum Flags {
                eBuffer, eImage
            };
            Flags flags;
            // buffer
            vkBuffer_sptr *bufSptr;
            bool unmap{false};
            vk::DeviceSize deviceSize{};
            const void* dataPtr{};
            MemoryUsage usage{MemoryUsage::eCpuToGpu};
            vk::BufferUsageFlags bufferUsageFlags{};
            uint8_t init{0};
            // image
            vkImage_sptr *imgSptr;
            vk::Extent2D extent{2560, 1440};
            vk::ImageUsageFlags imageUsageFlags;
            uint8_t imageCount{1};

            // id
            std::string id;
        } au;

    public:
        template<typename T, typename ...Args>
        auto allocAuto(T&& t, Args&& ...args) -> void{
            using f = std::decay_t<T>;

            auto alloc = [&] {
                if (au.flags == AllocAuto::eBuffer) {
                    if (au.usage == MemoryUsage::eGpuOnly) {
                        if (au.id.empty()) {
                            *au.bufSptr = allocBufferStaging(au.deviceSize, au.dataPtr, au.bufferUsageFlags,
                                                             IdGenerator::uniqueId());
                        } else {
                            *au.bufSptr = allocBufferStaging(au.deviceSize, au.dataPtr, au.bufferUsageFlags, au.id);
                        }
                    } else {
                        if (au.id.empty()) {
                            *au.bufSptr = allocBuffer(au.deviceSize, au.dataPtr, au.bufferUsageFlags, au.usage,
                                                      IdGenerator::uniqueId(), au.unmap);
                        } else {
                            *au.bufSptr = allocBuffer(au.deviceSize, au.dataPtr, au.bufferUsageFlags, au.usage, au.id,
                                                      au.unmap);
                        }
                    }
                } else if (au.flags == AllocAuto::eImage) {
                    if (au.imageUsageFlags & vk::ImageUsageFlagBits::eStorage && au.id.empty()) {
                        if (au.id.empty()) {
                            *au.imgSptr = allocImage(ImageConfig()
                                                             .setExtent(au.extent)
                                                             .setImageFlags(ImageFlags::eStorage)
                                                             .setUsage(vk::ImageUsageFlagBits::eStorage)
                                                             .setImageCount(au.imageCount));
                        } else {
                            *au.imgSptr = allocImage(ImageConfig()
                                                             .setExtent(au.extent)
                                                             .setImageFlags(ImageFlags::eStorage)
                                                             .setUsage(vk::ImageUsageFlagBits::eStorage)
                                                             .setImageCount(au.imageCount), au.id);
                        }
                    } else {

                    }
                }
                au.id.clear();
            };

            if constexpr (std::is_same_v<f, vkBuffer_sptr>){
                if (au.init == 1){
                    alloc();
                }
                au.bufSptr = &t;
                au.init = 1;
                au.flags = AllocAuto::eBuffer;
            } else if constexpr (std::is_same_v<f, vkImage_sptr>){
                if (au.init == 1) {
                    alloc();
                }
                au.imgSptr = &t;
                au.init = 1;
                au.flags = AllocAuto::eImage;
            } else if constexpr (std::is_same_v<f, bool>){
                au.unmap = t;
            } else if constexpr (std::is_same_v<f, MemoryUsage>){
                au.usage = t;
            } else if constexpr (std::is_same_v<f, vk::BufferUsageFlags> || std::is_same_v<f, vk::BufferUsageFlagBits>){
                au.bufferUsageFlags = t;
            } else if constexpr (std::is_pointer_v<f>){
                au.dataPtr = t;
            } else if constexpr (std::is_same_v<f, vk::DeviceSize>){
                au.deviceSize = t;
            } else if constexpr (std::is_same_v<f, vk::Extent2D>) {
                au.extent = t;
            } else if constexpr (std::is_integral_v<f>) {
                au.imageCount = t;
            } else if constexpr (std::is_same_v<f, vk::ImageUsageFlagBits> || std::is_same_v<f, vk::ImageUsageFlags>) {
                au.imageUsageFlags = t;
            } else if constexpr (std::is_same_v<f, std::string>){
                au.id = t;
            } else {
                using actualType = std::remove_reference_t<T>;
                au.dataPtr = getBufData(t);
                au.deviceSize = t.size() * sizeof (typename actualType::value_type);
            }

            if constexpr (sizeof ...(args) > 0){
                allocAuto(std::forward<Args>(args)...);
            } else {
                alloc();
                au.bufSptr = nullptr;
                au.imgSptr = nullptr;
                au.deviceSize = 0;
                au.dataPtr = nullptr;
                au.usage = MemoryUsage::eCpuToGpu;
                au.flags = {};
                au.init = 0;
                au.bufferUsageFlags = {};
                au.imageUsageFlags = {};
                au.extent = vk::Extent2D{2560, 1440};
                au.imageCount = 1;
            }
        }

    public:
        auto clear() -> void{
            destroyStagBuf();
            CommandBufferCoordinator::clear();
        }
        auto getAccelAddr(const vkAccel_sptr& as) -> vk::DeviceAddress;
        auto getBufAddr(const vkBuf_sptr& buf) -> vk::DeviceAddress;

        template<typename T>
        auto getDeviceAddress(const T& t) -> vk::DeviceAddress {
            using type = std::decay_t<T>;
            if constexpr (std::is_same_v<type, vkBuffer_sptr>){
                return getBufAddr(t);
            } else if constexpr (std::is_same_v<type, vkAccel_sptr>){
                return getAccelAddr(t);
            }
        };
        auto glmMatToVkTransformMatrix(const glm::mat4& mat = glm::mat4 (1.f)) -> vk::TransformMatrixKHR;
        auto transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) -> void;
        auto transitionImageLayout(vk::ImageMemoryBarrier barrier,
                                   const vk::PipelineStageFlags& srcStage,
                                   const vk::PipelineStageFlags& dstStag) -> void;
        auto transitionImageLayout(vk::ImageMemoryBarrier barrier,
                                   const vk::PipelineStageFlags& srcStage,
                                   const vk::PipelineStageFlags& dstStag,
                                   vk::CommandBuffer& cmd) -> void;
        auto copyImageToImage(const vkImage_sptr& srcImage, vk::ImageLayout srcImageLayout,
                              const vkImage_sptr& dstImage, vk::ImageLayout dstImageLayout,
                              uint8_t copyImageIndex = 0) -> void;
        auto copyImageToImage(const vkImage_sptr& srcImage, vk::ImageLayout srcImageLayout,
                              const vkImage_sptr& dstImage, vk::ImageLayout dstImageLayout,
                              vk::CommandBuffer& cmd, uint8_t copyImageIndex = 0) -> void;
        auto copyImageToImage(const yic2::Image_sptr& srcImage, const yic2::Image_sptr& dstImage,
                              vk::CommandBuffer& cmd, uint8_t copyImageIndex = 0) -> void;

        template<typename ...Args>
        auto pipelineBarrier2(vk::CommandBuffer& cmd, vk::DependencyInfo info, Args&&...args){
            auto processArgs = []<typename T>(
                    vot::vector<vk::ImageMemoryBarrier2>& imageBarriers,
                             vot::vector<vk::BufferMemoryBarrier2>& bufferBarriers,
                             vot::vector<vk::MemoryBarrier2>& memoryBarriers,
                             T&& first){
                using type = std::decay_t<T>;
                if constexpr (std::is_same_v<type, vk::ImageMemoryBarrier2>){
                    imageBarriers.emplace_back(std::forward<T>(first));
                } else if constexpr (std::is_same_v<type, vk::BufferMemoryBarrier2>){
                    bufferBarriers.emplace_back(std::forward<T>(first));
                } else if constexpr (std::is_same_v<type, vk::MemoryBarrier2>){
                    memoryBarriers.emplace_back(std::forward<T>(first));
                }
            };

            vot::vector<vk::ImageMemoryBarrier2> imageBarrier2s;
            vot::vector<vk::BufferMemoryBarrier2> bufferBarrier2s;
            vot::vector<vk::MemoryBarrier2> memoryBarrier2s;

            (processArgs(imageBarrier2s, bufferBarrier2s, memoryBarrier2s, std::forward<Args>(args)), ...);

            pipelineBarrier2(imageBarrier2s, bufferBarrier2s, memoryBarrier2s, cmd, info);
        }

    private:
        auto pipelineBarrier2(const vot::vector<vk::ImageMemoryBarrier2>& imageMemoryBarrier2,
                              const vot::vector<vk::BufferMemoryBarrier2>& bufferMemoryBarrier2,
                              const vot::vector<vk::MemoryBarrier2>& memoryBarrier2, vk::CommandBuffer& cmd,
                              vk::DependencyInfo dependencyInfo = {}) -> void;
        auto createBuf(const bufCreateInfo& bufCreateInfo) -> BufferHandle;
        auto mapBuf(const BufferHandle& buf, VkDeviceSize devSize, const void* data, bool unmap) -> void*;
        inline static auto copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize, vk::CommandBuffer& cmd) -> void;

        inline static auto copyBufToImg(VkBuffer buf, VkImage img, uint32_t w, uint32_t h, vk::CommandBuffer& cmd) -> void;

        auto createImage(const yic2::ImageConfig& config) -> std::pair<VkImage, VmaAllocation>;
        auto createImageView(const yic2::ImageConfig& config, const vk::Image& image) -> vk::ImageView;
        inline static auto transitionImageLayout(VkImage image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer& cmd) -> void;

        inline static bool hasFlag(vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> item, vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> flag) { return (item & flag) == flag; }
    private:
        ev::pVkSetupContext ct{};

        vk::Device mDevice;
        vk::DispatchLoaderDynamic mDyDispatcher;

        VmaAllocator mVmaAllocator{};

    private: // staging buf manager
        struct stagBuf{
            BufferHandle stgBuf{};
            vk::DeviceSize deviceSize{};
        };

        auto acquireStagingBuf(vk::DeviceSize deviceSize) -> stagBuf;
        auto releaseStagingBuf(stagBuf stg) -> void;
        auto destroyStagBuf() -> void;
        inline static auto resetBuf(stagBuf& buf, vk::CommandBuffer& cmd) -> void;

        std::atomic<uint8_t> mStagBufCounter{};
        std::atomic<uint8_t> mDestroyCount{};
        oneapi::tbb::concurrent_map<vk::DeviceSize , oneapi::tbb::concurrent_queue<stagBuf>> mStagingBufs;
    };

} // yic

namespace mg{
    inline yic::Allocator* Allocator;
}

#endif //VKCELSHADINGRENDERER_VKALLOCATOR_H
