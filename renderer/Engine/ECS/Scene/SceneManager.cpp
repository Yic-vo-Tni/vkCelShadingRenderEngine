//
// Created by lenovo on 10/29/2024.
//

#include "SceneManager.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"

namespace sc {


    SceneManager::SceneManager() : ct(yic::systemHub.val<ev::pVkSetupContext>()){
        mRtOffImage = yic::allocator->allocImage(vot::ImageCI()
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "RayTracing off Image");


    }
} // sc