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
using vkBuffer_sptr = std::shared_ptr<yic::vkBuffer>;
using vkImg_sptr = std::shared_ptr<yic::vkImage>;
using vkImage_sptr = std::shared_ptr<yic::vkImage>;
using vkAccel_sptr = std::shared_ptr<yic::vkAccel>;

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

        struct buf{
            VkBuffer buf;
            VmaAllocation vmaAllocation;
        };

        struct img{
            VkImage img{};
            VmaAllocation vmaAllocation{};
            vk::ImageView imgView;
        };

    public:
        vkGet auto make = [](){ return Singleton<Allocator>::get(); };
        Allocator();
        ~Allocator();

//        auto make() -> void;

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
            } else if constexpr (std::is_same_v<f, vk::BufferUsageFlags>){
                au.bufferUsageFlags = t;
            } else if constexpr (std::is_pointer_v<f>){
                au.dataPtr = t;
            } else if constexpr (std::is_same_v<f, vk::DeviceSize>){
                au.deviceSize = t;
            } else if constexpr (std::is_same_v<f, vk::Extent2D>) {
                au.extent = t;
            } else if constexpr (std::is_integral_v<f>) {
                au.imageCount = t;
            } else if constexpr (std::is_same_v<f, vk::ImageUsageFlagBits>) {
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
//        template<typename T, typename ...Args>
//        static void allocBufAuto(T&& t, Args&&...args){
//            auto& bufAuto = get()->bufAuto;
//            using f = std::decay_t<T>;
////            std::cout << "Type of T: " << typeid(T).name() << ", value: \n";
////            std::cout << to_string(bufAuto.flags) << std::endl;
//            if constexpr (std::is_same_v<f, vkBuf_sptr>){
//                if (bufAuto.init == 1){
//                    *bufAuto.bufSptr = allocBuf(bufAuto.deviceSize, bufAuto.dataPtr, bufAuto.flags, bufAuto.usage, IdGenerator::uniqueId(), bufAuto.unmap);
//                }
//                bufAuto.bufSptr = &t;
//                bufAuto.init = 1;
//            } else if constexpr (std::is_same_v<f, bool>){
//                bufAuto.unmap = t;
//            } else if constexpr (std::is_same_v<f, MemoryUsage>){
//                bufAuto.usage = t;
//            } else if constexpr (std::is_same_v<f, vk::BufferUsageFlags>){
//                bufAuto.flags = t;
//            } else if constexpr (std::is_pointer_v<f>){
//                bufAuto.dataPtr = t;
//            } else if constexpr (std::is_same_v<f, vk::DeviceSize> || std::is_integral_v<f> || std::is_same_v<f, size_t>){
//                bufAuto.deviceSize = t;
//            } else {
//                using ActualType = std::remove_reference_t<T>;
//                bufAuto.dataPtr = getBufData(t);
//                bufAuto.deviceSize = t.size() * sizeof(typename ActualType::value_type);
//            }
//
//            if constexpr (sizeof ...(args) > 0){
//                allocBufAuto(std::forward<Args>(args)...);
//            } else {
//                *bufAuto.bufSptr = allocBuf(bufAuto.deviceSize, bufAuto.dataPtr, bufAuto.flags, bufAuto.usage,
//                                            IdGenerator::uniqueId(), bufAuto.unmap);
//                bufAuto.bufSptr = nullptr;
//                bufAuto.unmap = false;
//                bufAuto.deviceSize = 0;
//                bufAuto.dataPtr = nullptr;
//                bufAuto.usage = MemoryUsage::eCpuToGpu;
//                bufAuto.flags = {};
//                bufAuto.init = 0;
//            }
//        }
//
//        template<typename T, typename ...Args>
//        static void allocImgAuto(T&& t, Args&&...args){
//            auto& imgAuto = get()->imgAuto;
//            using f = std::decay_t<T>;
//
//            if constexpr (std::is_same_v<f, vkImg_sptr>){
//                if (imgAuto.init == 1){
//                    if ((imgAuto.flags & vk::ImageUsageFlagBits::eStorage)){
//                        *imgAuto.imgSptr = allocImgOffScreen_Storage(imgAuto.extent, imgAuto.imageCount);
//                    }
//                }
//                imgAuto.imgSptr = &t;
//                imgAuto.init = 1;
//            } else if constexpr (std::is_same_v<f, vk::Extent2D>){
//                imgAuto.extent = t;
//            } else if constexpr (std::is_integral_v<f>){
//                imgAuto.imageCount = t;
//            } else if constexpr (std::is_same_v<f, vk::ImageUsageFlagBits>){
//                imgAuto.flags = t;
//            }
//
//            if constexpr (sizeof ...(args) > 0){
//                allocImgAuto(std::forward<Args>(args)...);
//            } else {
//                if ((imgAuto.flags & vk::ImageUsageFlagBits::eStorage)){
//                    *imgAuto.imgSptr = allocImgOffScreen_Storage(imgAuto.extent, imgAuto.imageCount);
//                }
//
//                imgAuto.imgSptr = nullptr;
//                imgAuto.extent = vk::Extent2D{800, 600};
//                imgAuto.flags = {};
//                imgAuto.init = {0};
//                imgAuto.imageCount = {1};
//            }
//        }

    public:
        auto clear() -> void{
            destroyStagBuf();
            CommandBufferCoordinator::clear();
        }
        auto getAccelDevAddr(const vkAccel_sptr& as) -> vk::DeviceSize {
            vk::AccelerationStructureDeviceAddressInfoKHR addrInfo{as->accel};
            return mDevice.getAccelerationStructureAddressKHR(addrInfo, mDyDispatcher);
        }
        auto getBufAddr(const vkBuf_sptr& buf) -> vk::DeviceAddress;
        static auto glmMatToVkTransformMatrix(const glm::mat4& mat = glm::mat4 (1.f)) -> vk::TransformMatrixKHR{
            vk::TransformMatrixKHR tfMatrix;
            auto temp = glm::transpose(mat);
            memcpy(&tfMatrix, &temp, sizeof(vk::TransformMatrixKHR));
            return tfMatrix;
        }
    private:
        auto createBuf(const bufCreateInfo& bufCreateInfo) -> buf;
        auto mapBuf(const buf& buf, VkDeviceSize devSize, const void* data, bool unmap) -> void*;
        inline static auto copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize, vk::CommandBuffer& cmd) -> void;

        inline static auto copyBufToImg(VkBuffer buf, VkImage img, uint32_t w, uint32_t h, vk::CommandBuffer& cmd) -> void;
        inline static auto transitionImageLayout(VkImage image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer& cmd) -> void;

        inline static bool hasFlag(vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> item, vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> flag) { return (item & flag) == flag; }
    private:
        ev::pVkSetupContext ct{};
        et::vkRenderContext rt;

        vk::Device mDevice;
        vk::DispatchLoaderDynamic mDyDispatcher;

        VmaAllocator mVmaAllocator{};

    private: // staging buf manager
        struct stagBuf{
            buf stgBuf{};
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
