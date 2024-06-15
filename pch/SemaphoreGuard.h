//
// Created by lenovo on 6/14/2024.
//

#ifndef VKCELSHADINGRENDERER_SEMAPHOREGUARD_H
#define VKCELSHADINGRENDERER_SEMAPHOREGUARD_H

#include "pch.h"

class SemaphoreGuard{
public:
    explicit SemaphoreGuard(std::binary_semaphore& semaphore) : sem(semaphore){
        sem.acquire();
    }

    ~SemaphoreGuard(){
        sem.release();
    }

private:
    std::binary_semaphore& sem;
};

#endif //VKCELSHADINGRENDERER_SEMAPHOREGUARD_H
