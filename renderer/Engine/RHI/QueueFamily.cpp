//
// Created by lenovo on 9/25/2024.
//

#include "QueueFamily.h"

namespace rhi {

    QueueFamily::QueueFamily() = default;

    auto QueueFamily::init(vk::PhysicalDevice phy) -> void {
        mPhy = phy;
        qFamilies.resize(static_cast<uint32_t>(vot::queueType::eCount));
        for(auto i = 0; i < static_cast<uint32_t>(vot::queueType::eCount); i++){
            mQueuingRwMutex.emplace_back(std::make_shared<oneapi::tbb::queuing_rw_mutex>());
        }

        qFamilies[static_cast<uint32_t>(vot::queueType::eGraphics)] = {.familyIndex = findQueueFamily(vot::queueType::eGraphics)};
        qFamilies[static_cast<uint32_t>(vot::queueType::eTransfer)] = {.familyIndex = findQueueFamily(vot::queueType::eTransfer)};
    }

    auto QueueFamily::create(vk::Device device, const uint8_t &count) -> void {
        for(auto& f : qFamilies){
            f.createQueues(device, count, " queue");
        }
    }


    auto QueueFamily::findQueueFamily(vot::queueType type, bool print) -> std::optional<uint32_t> {
        auto families = mPhy.getQueueFamilyProperties();
        if (print) {
            if_debug { yic::logger->trace("physical device support {0} queue families! ", families.size()); }
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

        for(uint32_t i = 0; const auto& f : families){
            if (type == vot::queueType::eGraphics){
                if (f.queueFlags & vk::QueueFlagBits::eGraphics)
                    return i;
            }

            if (type == vot::queueType::eTransfer){
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

    bool QueueFamily::Family::createQueues(vk::Device device, uint32_t queueCount, const std::string &des) {
        if (familyIndex.has_value()) {
            queues.resize(queueCount);
            for (uint32_t i = 0; i < queueCount; i++) {
                queues[i] = device.getQueue(familyIndex.value(), i);
                if_debug yic::logger->trace("create " + std::to_string(i) + " " + des + " queue successfully");
            }
        }
        return true;
    }
} // rhi