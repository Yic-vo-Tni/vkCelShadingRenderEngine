//
// Created by lenovo on 10/29/2024.
//

#include "IndexManager.h"

namespace yic {

    IndexManager::IndexManager() {
        availableIndices.push(0);
    }

    IndexManager::IndexManager(uint32_t initIndex) : nextIndex(initIndex) {
        for(uint32_t i = 0; i < initIndex; ++i){
            availableIndices.push(i);
        }
    }

    auto IndexManager::allocate() -> uint32_t {
        if (availableIndices.empty()){
            return nextIndex++;
        } else {
            auto index = availableIndices.top();
            availableIndices.pop();
            return index;
        }
    }

    auto IndexManager::deallocate(uint32_t index) -> void {
        if (index < nextIndex)
            availableIndices.push(index);
    }

} // yic