//
// Created by lenovo on 6/13/2026.
//

#ifndef HF_DIAGNOSTICSTYPES_H
#define HF_DIAGNOSTICSTYPES_H


namespace vot::core {
    namespace diag {
        enum Lifetime {
            eVkBuffer,
            eVkImage,
            eVkImageView,
            eVkSampler,
            eVkPipeline,
            eVkPipelineLayout,
            eVkDescriptorSetLayout,
            eVkDescriptorPool,
            eVkFramebuffer,
            eVkRenderPass,
            eVkCommandPool,


            eThread,
            eWindow,
            eRuntimeSystem,
        };
    }


    namespace detail {
    }

    namespace api {
        struct LifetimeRecord {
            ::vot::core::diag::Lifetime lifetime;
            container::string name;
            void *handle = nullptr;
            std::source_location loc = std::source_location::current();
        };
    }
}

#endif //HF_DIAGNOSTICSTYPES_H
