//
// Created by lenovo on 1/14/2026.
//

#ifndef VKCELSHADINGRENDERER_THREADSAFELRUCACHE_H
#define VKCELSHADINGRENDERER_THREADSAFELRUCACHE_H

namespace vot::gfx {
    template<typename Key, typename Value>
    class ThreadSafeLRUCache {
        struct Node {
            Key key;
            Value value;
        };

        size_t capacity;
        std::list<Node> lruList;
        std::unordered_map<Key, typename std::list<Node>::iterator> map;
        std::function<void(Value &)> destroyFn;
        mutable std::shared_mutex mutex;
        std::atomic<int> activeCount{0};

    public:
        explicit ThreadSafeLRUCache(const size_t cap, const std::function<void(Value &)> &destroyFn) : capacity(cap),
            destroyFn(destroyFn) {
        }

        ~ThreadSafeLRUCache() { clear(); }

        bool get(const Key &key, Value &out) {
            std::unique_lock lock(mutex);
            auto it = map.find(key);
            if (it == map.end()) return false;
            lruList.splice(lruList.begin(), lruList, it->second);
            out = it->second->value;
            return true;
        }

        void put(const Key &key, Value val) {
            std::unique_lock lock(mutex);
            auto it = map.find(key);

            if (it != map.end()) {
                destroyFn(it->second->value);
                lruList.erase(it->second);
                map.erase(it);
            }

            if (lruList.size() >= capacity) {
                auto &back = lruList.back();
                destroyFn(back.value);
                lruList.pop_back();
                map.erase(back.key);
            }

            lruList.push_front({key, std::move(val)});
            map[key] = lruList.begin();
        }

        void clear() {
            std::unique_lock lock(mutex);
            while (!lruList.empty()) {
                auto it = std::prev(lruList.end());
                destroyFn(it->value);
                lruList.erase(it);
            }
            map.clear();
        }

        int getActiveCount() const {
            return activeCount.load();
        }
    };


    class LRUStagingBufferCache {
    public:
        explicit LRUStagingBufferCache(VmaAllocator &allocator)
            : vmaAllocator(allocator), caches(32, [&](const detail::StagingBufferAllocation &allocation) {
                vmaDestroyBuffer(vmaAllocator, std::get<0>(allocation), std::get<1>(allocation));
            }) {
        }
        ~LRUStagingBufferCache() {
            caches.clear();
        };

        auto acquire(const vk::DeviceSize& device_size) -> detail::StagingBufferAllocation {
            detail::StagingBufferAllocation allocation;
            if (!caches.get(device_size, allocation)) {
                detail::BufferAllocation buffer_allocation;

                const VkBufferCreateInfo bufferCI{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = device_size,
                    .usage = static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst),
                };
                constexpr VmaAllocationCreateInfo vmaCI{
                    .flags = static_cast<VmaAllocationCreateFlags>(api::AllocStrategy::eMapped),
                    .usage = static_cast<VmaMemoryUsage>(api::MemoryUsage::eCpuOnly),
                };

                if (vmaCreateBuffer(vmaAllocator, &bufferCI, &vmaCI, &buffer_allocation.first, &buffer_allocation.second, nullptr) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create buffer");
                }

                allocation = {buffer_allocation.first, buffer_allocation.second, device_size};
                caches.put(device_size, allocation);
            }
            return allocation;
        };
    private:
        VmaAllocator& vmaAllocator;
        ThreadSafeLRUCache<vk::DeviceSize, detail::StagingBufferAllocation> caches;
    };
}

#endif //VKCELSHADINGRENDERER_THREADSAFELRUCACHE_H
