//
// Created by lenovo on 6/20/2024.
//

#ifndef VKCELSHADINGRENDERER_LOCKEDSTATE_H
#define VKCELSHADINGRENDERER_LOCKEDSTATE_H

#include "pch.h"

template<typename T>
class LockedState {
private:
    T &state;
    //oneapi::tbb::spin_rw_mutex::scoped_lock lock;
    oneapi::tbb::queuing_rw_mutex::scoped_lock lock;

public:
//    LockedState(T &state, oneapi::tbb::spin_rw_mutex &mutex)
//            : state(state), lock(mutex, true) {}
    LockedState(T &state, oneapi::tbb::queuing_rw_mutex &mutex)
            : state(state), lock(mutex, true) {}

    ~LockedState() {

    }

    LockedState &operator=(const LockedState &) = delete;


    T &get() {
        return state;
    }


    void release() {
        lock.release();
    }
};


#endif //VKCELSHADINGRENDERER_LOCKEDSTATE_H
