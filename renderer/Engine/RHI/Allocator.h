//
// Created by lenovo on 9/26/2024.
//

#ifndef VKCELSHADINGRENDERER_ALLOCATOR_H
#define VKCELSHADINGRENDERER_ALLOCATOR_H

#include "Buffer.h"
#include "Image.h"

namespace rhi {

    class Allocator {
        using bufferHandle = std::pair<VkBuffer, VmaAllocation>;
        using imageHandle = std::pair<VkImage, VmaAllocation>;
        using imagePath = std::variant<vot::string, vot::vector<vot::string>>;
        using stagingBufferHandle = std::tuple<VkBuffer, VmaAllocation, vk::DeviceSize>;
        struct BufferCI{
            vk::DeviceSize devSize;
            vk::BufferUsageFlags flags;
            vot::memoryUsage memoryUsage;
            vot::allocStrategy allocStrategy;
        };
    public:
        Make = []{ return Singleton<Allocator>::make_ptr(); };
        Allocator();
        ~Allocator() = default;

        auto clear() -> void;

        auto allocBuffer(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, vot::memoryUsage usage = vot::memoryUsage::eCpuToGpu,
                         const vot::string& id = IdGenerator::uniqueId(), bool unmap = false) -> vot::Buffer_sptr;
        auto allocBuffer(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, const vot::string& id) -> vot::Buffer_sptr;
        auto allocBuffer(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const vot::string& id) -> vot::Buffer_sptr ;
        auto allocBufferStaging(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, vot::memoryUsage usage = vot::memoryUsage::eGpuOnly,
                                vot::allocStrategy strategy = vot::allocStrategy::eMapped, const vot::string& id = IdGenerator::uniqueId()) -> vot::Buffer_sptr ;
        auto allocBufferStaging(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, const vot::string& id) -> vot::Buffer_sptr;
        auto allocBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const vot::string& id = IdGenerator::uniqueId()) -> vot::Buffer_sptr;
        auto allocDedicatedBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const vot::string& id = IdGenerator::uniqueId()) -> vot::Buffer_sptr ;

        auto allocAccel(vk::AccelerationStructureCreateInfoKHR& createInfoKhr) -> vot::Accel_sptr ;
        auto allocAccel(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfoKhr, vk::AccelerationStructureTypeKHR type) -> vot::Accel_sptr;

        auto loadTexture(const imagePath& pt) -> vot::Image_sptr;
        auto allocImage(vot::ImageCI config, const vot::string& id) -> vot::Image_sptr ;
    public:
        template<typename ...Args>
        auto pipelineBarrier2(vot::CommandBuffer& cmd, vk::DependencyInfo info, Args&&...args){
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
                } else if constexpr (std::is_same_v<type, vot::vector<vk::ImageMemoryBarrier2>>){
                    imageBarriers.insert(imageBarriers.end(), std::forward<T>(first).begin(), std::forward<T>(first).end());
                }
            };

            vot::vector<vk::ImageMemoryBarrier2> imageBarrier2s;
            vot::vector<vk::BufferMemoryBarrier2> bufferBarrier2s;
            vot::vector<vk::MemoryBarrier2> memoryBarrier2s;

            (processArgs(imageBarrier2s, bufferBarrier2s, memoryBarrier2s, std::forward<Args>(args)), ...);

            pipelineBarrier2I(imageBarrier2s, bufferBarrier2s, memoryBarrier2s, cmd, info);
        }

        auto glmMatToVkTransformMatrix(const glm::mat4 &mat = glm::mat4(1.f)) -> vk::TransformMatrixKHR {
            vk::TransformMatrixKHR tfMatrix;
            auto temp = glm::transpose(mat);
            memcpy(&tfMatrix, &temp, sizeof(vk::TransformMatrixKHR));
            return tfMatrix;
        }
    private:
        auto createBuffer(const BufferCI& ci) -> bufferHandle;
        auto mapBuffer(VmaAllocation& alloc, VkDeviceSize devSize, const void* data, bool unmap) -> void*;
        static auto copyBuffer(VkBuffer stagingBuffer, VkBuffer destBuffer, VkDeviceSize deviceSize, vot::CommandBuffer& cmd) -> void;
        static auto resetBuffer(VkBuffer& buffer, vot::CommandBuffer& cmd) -> void;

        auto createImage(const vot::ImageCI& config) -> imageHandle ;
        auto createImageView(const vot::ImageCI& config, const vk::Image& image) const -> vk::ImageView;
        inline auto copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t w, uint32_t h, vot::CommandBuffer& cmd) -> void;

        static auto pipelineBarrier2I(const vot::vector<vk::ImageMemoryBarrier2>& imageMemoryBarrier2,
                              const vot::vector<vk::BufferMemoryBarrier2>& bufferMemoryBarrier2,
                              const vot::vector<vk::MemoryBarrier2>& memoryBarrier2, vot::CommandBuffer& cmd,
                              vk::DependencyInfo dependencyInfo = {}) -> void;
    private:
        ev::pVkSetupContext ct{};
        VmaAllocator mVmaAllocator{};

    private:
        auto acquireStagingBuffer(vk::DeviceSize deviceSize) -> stagingBufferHandle;
        auto releaseStagingBuffer(stagingBufferHandle handle) -> void;
        std::atomic<uint8_t> mStagBufferCounter{};
        std::atomic<uint8_t> mDestroyCount{};
        oneapi::tbb::concurrent_map<vk::DeviceSize , oneapi::tbb::concurrent_queue<stagingBufferHandle>> mStagingBuffers;
    };

} // r// hi

namespace yic{
    inline rhi::Allocator* allocator;
}

#endif //VKCELSHADINGRENDERER_ALLOCATOR_H
