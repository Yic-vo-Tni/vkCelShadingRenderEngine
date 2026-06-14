//
// Created by lenovo on 6/13/2026.
//

#include "LifetimeTracker.h"

namespace vot::core::instance {
    auto LifetimeTracker::add(const api::LifetimeRecord &lifetime_record) -> void {
        if (lifetime_record.handle == nullptr) {
            return;
        }

        std::scoped_lock lock(mutex);

        mRecords[lifetime_record.handle] = lifetime_record;
    }

    auto LifetimeTracker::remove(void *handle) -> void {
        if (handle == nullptr) {return;}

        std::scoped_lock lock(mutex);
        mRecords.erase(handle);
    }

    auto LifetimeTracker::empty() const -> bool {
        std::scoped_lock lock(mutex);
        return mRecords.empty();
    }

    auto LifetimeTracker::size() const -> std::size_t {
        std::scoped_lock lock(mutex);
        return mRecords.size();
    }

    auto LifetimeTracker::dump() const -> void {
        std::scoped_lock lock(mutex);

        if (mRecords.empty()) {
            yic::logger->info("[LifetimeTracker] all tracked resources destroyed.");
            return;
        }

        yic::logger->error("[LifetimeTracker] leaked resources: {}", mRecords.size());

        for (const auto& [handle, record] : mRecords) {
            yic::logger->error(
                "[LifetimeTracker] leak: name={}, handle={}, file={}, line={}, function={}",
                record.name,
                record.handle,
                record.loc.file_name(),
                record.loc.line(),
                record.loc.function_name()
            );
        }
    }


    auto lifetimeTracker() -> LifetimeTracker& {
        static auto tracker = LifetimeTracker();
        return tracker;
    };

}
