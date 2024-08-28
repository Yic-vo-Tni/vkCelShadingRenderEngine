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
using vkImg_sptr = std::shared_ptr<yic::vkImage>;
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

        struct BLASInput{
            std::vector<vk::AccelerationStructureGeometryKHR> asGeometry;
            std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildRangeInfo;
            vk::BuildAccelerationStructureFlagsKHR flags{0};
        };
        struct BuildAccelerationStructure{
            vk::AccelerationStructureBuildGeometryInfoKHR geoInfo{};
            vk::AccelerationStructureBuildSizesInfoKHR sizeInfo{};
            const vk::AccelerationStructureBuildRangeInfoKHR* rangeInfo{};
            vkAccel_sptr asSptr;
        };
        static uint32_t mMainRenderImageCount;
    public:
        vkGet auto get = [](){ return Singleton<Allocator>::get(); };
        Allocator();
        ~Allocator();

        DEFINE_STATIC_ACCESSOR_PARAM(allocBuf,
                               (vk::DeviceSize ds, vk::BufferUsageFlags fg, MemoryUsage us, const std::string& id = IdGenerator::uniqueId()),
                               (ds, nullptr, fg, us, id, false));

        DEFINE_STATIC_ACCESSOR_PARAM(allocBuf,
                    (vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags, MemoryUsage usage = MemoryUsage::eCpuToGpu, const std::string& id = IdGenerator::uniqueId(), bool unmap = false),
                    (deviceSize, data, flags, usage, id, unmap));

        DEFINE_STATIC_ACCESSOR_PARAM(allocBufStaging,
                               (vk::DeviceSize ds, const void* data, vk::BufferUsageFlags fg, MemoryUsage us = MemoryUsage::eGpuOnly, const std::string& id = IdGenerator::uniqueId()),
                               (ds, data, fg, id, us));
        DEFINE_STATIC_ACCESSOR_PARAM(allocBufStaging,
                                     (vk::DeviceSize ds, vk::BufferUsageFlags fg, MemoryUsage us = MemoryUsage::eGpuOnly, const std::string& id = IdGenerator::uniqueId()),
                                     (ds, nullptr, fg, id, us));

        DEFINE_STATIC_ACCESSOR_PARAM(allocImg,
                               (const imgPath& pt),
                               (pt));

        DEFINE_STATIC_ACCESSOR_PARAM(allocImg,
                               (const imgPath& pt, vkImageConfig cf),
                               (pt, cf));

        DEFINE_STATIC_ACCESSOR_PARAM(allocImgOffScreen,
                               (const vkImageConfig& cf, const std::string& id, size_t count),
                               (cf, id, count));

        DEFINE_STATIC_CUSTOM_ACCESSOR(allocImgOffScreen_DepthStencil, allocImgOffScreen_impl,
                                      (vkImageConfig cf, const std::string& id, size_t count),
                                      (cf.setImageFlags(static_cast<vkImageConfig::ImageFlags>(vkImageConfig::eColor | vkImageConfig::eDepth)), id, count));

        DEFINE_STATIC_CUSTOM_ACCESSOR(allocImgOffScreen_DepthStencilAndFramebuffers, allocImgOffScreen_impl,
                                      (vkImageConfig cf, vk::RenderPass rp, const std::string& id, size_t count),
                                      (cf.setImageFlags(static_cast<vkImageConfig::ImageFlags>(vkImageConfig::eColor | vkImageConfig::eDepth)).setRenderPass(rp), id, count));

        DEFINE_STATIC_CUSTOM_ACCESSOR(allocImgOffScreen_Storage, allocImgOffScreen_impl,
                                      (vk::Extent2D extent, size_t count = mMainRenderImageCount, const std::string& id = IdGenerator::uniqueId()),
                                      (vkImageConfig{extent}.setImageFlags(vkImageConfig::eStorage).setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled), id, count));

        DEFINE_STATIC_ACCESSOR_PARAM(buildBLAS,
                               (const std::vector<BLASInput> &inputs, vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace),
                               (inputs, flags));
        DEFINE_STATIC_ACCESSOR_PARAM(buildBLAS,
                                     (const BLASInput &input, vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace),
                                     ({ input }, flags));

        DEFINE_STATIC_ACCESSOR_PARAM(buildTLAS,
                                     (const std::vector<vk::AccelerationStructureInstanceKHR> &instances,
                                             vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, bool update = false),
                                     (instances, flags, update));
        DEFINE_STATIC_ACCESSOR_PARAM(buildTLAS,
                                     (const vk::AccelerationStructureInstanceKHR &instance,
                                             vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, bool update = false),
                                     ({ instance }, flags, update));

        DEFINE_STATIC_ACCESSOR_PARAM(modelToGeometryKHR,
                                     (const sc::Model::Generic& model),
                                     (model));

        DEFINE_STATIC_ACCESSOR_PARAM(allocAccel,
                                     (vk::AccelerationStructureCreateInfoKHR& inf),
                                     (inf));

/// AUTO
    private:
        struct AllocBufAuto {
            vkBuf_sptr *bufSptr;
            bool unmap{false};
            vk::DeviceSize deviceSize{};
            const void *dataPtr{};
            MemoryUsage usage{MemoryUsage::eCpuToGpu};
            vk::BufferUsageFlags flags{};
            uint8_t init{0};
        };
        struct AllocImgAuto{
            vkImg_sptr *imgSptr;
            vk::Extent2D extent{800, 600};
            vk::ImageUsageFlags flags;
            uint8_t init{0};
            uint8_t imageCount{1};
        };
        AllocBufAuto bufAuto;
        AllocImgAuto imgAuto;

    public:
        template<typename T, typename ...Args>
        static void allocBufAuto(T&& t, Args&&...args){
            auto& bufAuto = get()->bufAuto;
            using f = std::decay_t<T>;
//            std::cout << "Type of T: " << typeid(T).name() << ", value: \n";
//            std::cout << to_string(bufAuto.flags) << std::endl;
            if constexpr (std::is_same_v<f, vkBuf_sptr>){
                if (bufAuto.init == 1){
                    *bufAuto.bufSptr = allocBuf(bufAuto.deviceSize, bufAuto.dataPtr, bufAuto.flags, bufAuto.usage, IdGenerator::uniqueId(), bufAuto.unmap);
                }
                bufAuto.bufSptr = &t;
                bufAuto.init = 1;
            } else if constexpr (std::is_same_v<f, bool>){
                bufAuto.unmap = t;
            } else if constexpr (std::is_same_v<f, MemoryUsage>){
                bufAuto.usage = t;
            } else if constexpr (std::is_same_v<f, vk::BufferUsageFlags>){
                bufAuto.flags = t;
            } else if constexpr (std::is_pointer_v<f>){
                bufAuto.dataPtr = t;
            } else if constexpr (std::is_same_v<f, vk::DeviceSize> || std::is_integral_v<f> || std::is_same_v<f, size_t>){
                bufAuto.deviceSize = t;
            } else {
                using ActualType = std::remove_reference_t<T>;
                bufAuto.dataPtr = getBufData(t);
                bufAuto.deviceSize = t.size() * sizeof(typename ActualType::value_type);
            }

            if constexpr (sizeof ...(args) > 0){
                allocBufAuto(std::forward<Args>(args)...);
            } else {
                *bufAuto.bufSptr = allocBuf(bufAuto.deviceSize, bufAuto.dataPtr, bufAuto.flags, bufAuto.usage,
                                            IdGenerator::uniqueId(), bufAuto.unmap);
                bufAuto.bufSptr = nullptr;
                bufAuto.unmap = false;
                bufAuto.deviceSize = 0;
                bufAuto.dataPtr = nullptr;
                bufAuto.usage = MemoryUsage::eCpuToGpu;
                bufAuto.flags = {};
                bufAuto.init = 0;
            }
        }

        template<typename T, typename ...Args>
        static void allocImgAuto(T&& t, Args&&...args){
            auto& imgAuto = get()->imgAuto;
            using f = std::decay_t<T>;

            if constexpr (std::is_same_v<f, vkImg_sptr>){
                if (imgAuto.init == 1){
                    if ((imgAuto.flags & vk::ImageUsageFlagBits::eStorage)){
                        *imgAuto.imgSptr = allocImgOffScreen_Storage(imgAuto.extent, imgAuto.imageCount);
                    }
                }
                imgAuto.imgSptr = &t;
                imgAuto.init = 1;
            } else if constexpr (std::is_same_v<f, vk::Extent2D>){
                imgAuto.extent = t;
            } else if constexpr (std::is_integral_v<f>){
                imgAuto.imageCount = t;
            } else if constexpr (std::is_same_v<f, vk::ImageUsageFlagBits>){
                imgAuto.flags = t;
            }

            if constexpr (sizeof ...(args) > 0){
                allocImgAuto(std::forward<Args>(args)...);
            } else {
                if ((imgAuto.flags & vk::ImageUsageFlagBits::eStorage)){
                    *imgAuto.imgSptr = allocImgOffScreen_Storage(imgAuto.extent, imgAuto.imageCount);
                }

                imgAuto.imgSptr = nullptr;
                imgAuto.extent = vk::Extent2D{800, 600};
                imgAuto.flags = {};
                imgAuto.init = {0};
                imgAuto.imageCount = {1};
            }
        }

    public:
        static auto clear() -> void{
            get()->destroyStagBuf();
            CommandBufferCoordinator::clear();
        }
        static auto getAccelDevAddr(const vkAccel_sptr& as) -> vk::DeviceSize {
            vk::AccelerationStructureDeviceAddressInfoKHR addrInfo{as->accel};
            return get()->mDevice.getAccelerationStructureAddressKHR(addrInfo, get()->mDyDispatcher);
        }
        static auto getBufAddr(const vkBuf_sptr& buf) -> vk::DeviceAddress;
        static auto glmMatToVkTransformMatrix(const glm::mat4& mat = glm::mat4 (1.f)) -> vk::TransformMatrixKHR{
            vk::TransformMatrixKHR tfMatrix;
            auto temp = glm::transpose(mat);
            memcpy(&tfMatrix, &temp, sizeof(vk::TransformMatrixKHR));
            return tfMatrix;
        }
    private:
        auto allocBuf_impl(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, MemoryUsage usage, const std::string& id, bool unmap) -> vkBuf_sptr;
        auto allocBufStaging_impl(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, const std::string& id,
                                   MemoryUsage usage = MemoryUsage::eGpuOnly,  AllocStrategy allocStrategy = AllocStrategy::eMapped) -> vkBuf_sptr;
        auto createBuf(const bufCreateInfo& bufCreateInfo) -> buf;
        auto mapBuf(const buf& buf, VkDeviceSize devSize, const void* data, bool unmap) -> void*;
        inline static auto copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize, vk::CommandBuffer& cmd) -> void;

        auto allocImg_impl(const imgPath& imgPath, std::optional<vkImageConfig> config = vkImageConfig{0}, std::string id = {}) -> vkImg_sptr;
        auto allocImgOffScreen_impl(vkImageConfig config, const std::string& id, size_t count) -> vkImg_sptr;
        inline static auto copyBufToImg(VkBuffer buf, VkImage img, uint32_t w, uint32_t h, vk::CommandBuffer& cmd) -> void;
        inline static auto transitionImageLayout(VkImage image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer& cmd) -> void;

        auto buildBLAS_impl(const std::vector<BLASInput> &input, vk::BuildAccelerationStructureFlagsKHR flags) -> std::vector<vkAccel_sptr>;
        auto buildTLAS_impl(const std::vector<vk::AccelerationStructureInstanceKHR> &instances,
                            vk::BuildAccelerationStructureFlagsKHR flags, bool update) -> vkAccel_sptr;
        auto allocAccel_impl(vk::AccelerationStructureCreateInfoKHR& createInfo) -> vkAccel_sptr;
        inline static bool hasFlag(vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> item, vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> flag) { return (item & flag) == flag; }
        auto modelToGeometryKHR_impl(const sc::Model::Generic& model) -> BLASInput;
    private:
        et::vkSetupContext ct;
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

#endif //VKCELSHADINGRENDERER_VKALLOCATOR_H
