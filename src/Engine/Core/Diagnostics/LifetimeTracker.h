//
// Created by lenovo on 6/13/2026.
//

#ifndef HF_LIFETIMETRACKER_H
#define HF_LIFETIMETRACKER_H

namespace vot::core::instance {
    class LifetimeTracker {
    public:
        auto add(const api::LifetimeRecord& lifetime_record) -> void;
        auto remove(void* handle) -> void;
        auto empty() const -> bool;
        auto size() const -> std::size_t;
        auto dump() const -> void;
    private:
        mutable std::mutex mutex;
        container::unordered_map<void*, api::LifetimeRecord> mRecords;
    };

    auto lifetimeTracker() -> LifetimeTracker&;
}

#ifdef NDEBUG

#define HF_TRACK_RESOURCE(type_, handle_) ((void)0)

#define HF_UNTRACK_RESOURCE(handle_) ((void)0)

#else

#define HF_TRACK_RESOURCE(type_, handle_)                                      \
    vot::core::instance::lifetimeTracker().add(::vot::core::api::LifetimeRecord{ \
        .lifetime = type_,                                                       \
        .name = #handle_,                                                        \
        .handle = reinterpret_cast<void*>(handle_),                              \
        .loc = std::source_location::current()                                   \
    })

#define HF_UNTRACK_RESOURCE(handle_)                                            \
    vot::core::instance::lifetimeTracker().remove(                               \
        reinterpret_cast<void*>(handle_)                                         \
    )

#endif

#endif //HF_LIFETIMETRACKER_H
