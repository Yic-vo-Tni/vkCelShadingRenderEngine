//
// Created by lenovo on 12/29/2025.
//

#ifndef VKCELSHADINGRENDERER_DYNAMICEDITABLERENDERING_H
#define VKCELSHADINGRENDERER_DYNAMICEDITABLERENDERING_H

#include "RHI/Pipeline/ComputePipeline.h"
#include "RHI/Pipeline/GraphicsPipeline.h"
#include "RHI/Pipeline/RayTracingPipeline.h"

using PipelineHandle = std::variant<
    std::shared_ptr<rhi::GraphicsPipeline>,
    std::shared_ptr<rhi::ComputePipeline>,
    std::shared_ptr<rhi::RayTracingPipeline> >;

namespace runtime::flow {
    namespace DER {
        struct AlgorithmNode;

        template <typename R>
        using LambdaInvokeUint = std::function<R()>;

        template <typename R>
        struct FnHandle {
            LambdaInvokeUint<R> fn;
            LambdaInvokeUint<R> last_good_fn;
            bool valid = false;
        };

        struct AlgorithmNode {
            std::uint32_t id;
            vot::string name;
            LambdaInvokeUint<vot::Image_sptr> buildTarget;
            LambdaInvokeUint<PipelineHandle> buildPipeline;
            LambdaInvokeUint<vot::DescriptorHandle> buildDescriptor;
            std::function<void(vot::CommandBuffer&)> dispatch;

            vot::vector<vot::Image_sptr> reads;
            vot::Image_sptr THandle;
            PipelineHandle PHandle;
            vot::DescriptorHandle DHandle;

            template<typename T>
            std::shared_ptr<T> *tryPipeline() {
                return std::get_if<std::shared_ptr<T> >(&PHandle);
            }

            template<typename T>
            std::shared_ptr<T> &pipeline() {
                return std::get<std::shared_ptr<T> >(PHandle);
            }

            template<typename T>
            const std::shared_ptr<T>* tryPipeline() const {
                return std::get_if<std::shared_ptr<T>>(&PHandle);
            }
            template<typename T>
            const std::shared_ptr<T>& pipeline() const {
                return std::get<std::shared_ptr<T>>(PHandle);
            }

            [[nodiscard]] auto makePassNode() const -> std::variant<vot::Image_sptr, vot::string> {
                if (THandle) return THandle;
                return name;
            }
        };
        struct AlgorithmFlow {
            vot::string name;
            vot::vector<AlgorithmNode> nodes;
        };
        struct RenderingFlow {
            vot::vector<AlgorithmFlow> algorithms{};
        };

        //////////////

        namespace detail {
            struct flow_t {};
            struct node_t {};

            template<typename F>
            AlgorithmFlow operator|(flow_t, F &&f) {
                AlgorithmFlow result{};
                std::forward<F>(f)(result);
                return result;
            }

            template<typename F>
            AlgorithmNode operator|(node_t, F &&f) {
                AlgorithmNode result{};
                std::forward<F>(f)(result);
                return result;
            }
        }


        inline constexpr detail::flow_t flow{};
        inline constexpr  detail::node_t node{};

    }

    class DynamicEditableRendering {
    public:
        DynamicEditableRendering();
        ~DynamicEditableRendering();

        auto drawEditor() -> void;

        auto queryNode(const vot::string& name) const -> const DER::AlgorithmNode& {
            for (auto &algo: rf.algorithms) {
                for (auto &node: algo.nodes) {
                    if (node.name == name) {
                        return node;
                    }
                }
            }
            throw std::runtime_error("DER::node view: node not found");
        }
        auto acquireRF() const { return rf;}
    private:
        std::uint32_t rfActive{0};
        std::uint32_t nodeId{0};
        DER::RenderingFlow rf{};
    };

    class DERTranslator {
    public:
        Make = [](const DynamicEditableRendering* der) { return Singleton<DERTranslator>::make_ptr(der); };
        explicit DERTranslator(const DynamicEditableRendering* der) : r(der) {}
        ~DERTranslator() = default;

        auto target(const vot::string &name) const -> vot::Image_sptr {
            return r->queryNode(name).THandle;
        }

        auto descriptor(const vot::string &name) const -> vot::DescriptorHandle {
            return r->queryNode(name).DHandle;
        }

        template<typename T>
        auto pipeline(const vot::string& name) const -> const std::shared_ptr<T>& {
            return r->queryNode(name).pipeline<T>();
        }

    private:
        const DynamicEditableRendering *r{};
    };
} // flow

namespace yic {
    inline runtime::flow::DERTranslator* derTranslator;
}

#endif //VKCELSHADINGRENDERER_DYNAMICEDITABLERENDERING_H








// namespace DER {
//     struct NodeRTConnectCI{};
//     struct AlgorithmInterface {
//         NodeRTConnectCI entry;
//         NodeRTConnectCI exit;
//     };
//     struct AlgorithmRTNodeCI {
//         vot::ImageCI ci;
//         NodeRTConnectCI entry;
//         NodeRTConnectCI exit;
//     };
//     struct AlgorithmFlow {
//         AlgorithmInterface algorithm_interface{};
//         vot::vector<AlgorithmRTNodeCI> nodes_ci{};
//     };
//
//     struct RenderingFlowCI {
//         std::uint32_t memory_size;
//     };
//
//     struct RenderingFlow {
//         RenderingFlowCI ci;
//         vot::vector<AlgorithmFlow> algorithms{};
//     };
// }