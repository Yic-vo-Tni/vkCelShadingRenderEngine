//
// Created by lenovo on 1/8/2026.
//

#ifndef VKCELSHADINGRENDERER_FORWARD_H
#define VKCELSHADINGRENDERER_FORWARD_H

#include "Runtime/System/DER/DynamicEditableRendering.h"

namespace runtime::flow {
        inline auto Compose_Target() -> vot::Image_sptr;
        inline auto Compose_Pipline() -> PipelineHandle;
        inline auto Compose_Descriptor() -> vot::DescriptorHandle;
        inline auto Compose_Dispatch(vot::CommandBuffer& cmd) -> void;
} // runtime

#endif //VKCELSHADINGRENDERER_FORWARD_H