//
// Created by lenovo on 9/25/2024.
//

#ifndef VKCELSHADINGRENDERER_QUEUEFAMILY_H
#define VKCELSHADINGRENDERER_QUEUEFAMILY_H

namespace rhi {

    class QueueFamily {
        struct Family{
            vot::vector<vk::Queue> queues{};
            std::optional<uint32_t> familyIndex{};
            bool createQueues(vk::Device device, uint32_t queueCount, const std::string& des);
        };
    public:
        Make = []{ return Singleton<QueueFamily>::make_ptr();};
        QueueFamily();

        auto init(vk::PhysicalDevice phy) -> void;
        auto create(vk::Device device, const uint8_t& count) -> void;
        auto getFamilies() { return qFamilies; }

        auto acquireQueue(vot::queueType type, uint8_t qCount = 0) {
            return Locked<vk::Queue, oneapi::tbb::queuing_rw_mutex>(qFamilies[static_cast<uint32_t>(type)].queues[qCount], *mQueuingRwMutex[static_cast<uint32_t>(type) * static_cast<uint32_t >(vot::queueType::eCount) + qCount]);
        };
        auto acquireLockedQueue(vot::queueType type, uint8_t qCount = 0) {
            return Locked<vk::Queue, oneapi::tbb::queuing_rw_mutex>(qFamilies[static_cast<uint32_t>(type)].queues[qCount], *mQueuingRwMutex[static_cast<uint32_t>(type) * static_cast<uint32_t >(vot::queueType::eCount) + qCount]);
        };
        auto acquireQueueUnSafe(vot::queueType type, uint8_t qCount = 0){
            return qFamilies[static_cast<uint32_t>(type)].queues[qCount];
        }
        auto acquireFamily(vot::queueType type){
            return qFamilies[static_cast<uint32_t>(type)];
        }
        auto acquireQueueIndex(vot::queueType type) ->  uint32_t {
            uint32_t r = qFamilies[static_cast<uint32_t>(type)].familyIndex.value_or(0);
            return r;
        }
    private:
        vk::PhysicalDevice mPhy;
        vot::vector<Family> qFamilies;
        vot::vector<std::shared_ptr<oneapi::tbb::queuing_rw_mutex>> mQueuingRwMutex;
        auto findQueueFamily(vot::queueType type, bool print = false) -> std::optional<uint32_t>;
    };

} // rhi

namespace yic{
    inline rhi::QueueFamily* qFamily;
}

#endif //VKCELSHADINGRENDERER_QUEUEFAMILY_H
