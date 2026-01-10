//
// Created by lenovo on 7/22/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERGRAPH_H
#define VKCELSHADINGRENDERER_RENDERGRAPH_H

#include "Runtime/System/RenderFlow/RenderTarget.h"

namespace runtime::flow {

    struct RenderPassNode{
        vot::string name;
        RT_sptr target;
        vot::vector<RT_sptr> inputs;
        vot::vector<RT_sptr> outputs;
        std::function<void(vot::CommandBuffer&)> execute;

        std::uint32_t order = 0;
        std::uint32_t layer = 0;
    };

    class RenderGraph {
    public:
        RenderGraph();
        auto addPass(const RenderPassNode& pass) -> void;

        auto begin() -> void{ passes.clear(); }
        auto end() -> void;
    private:
        auto compile() -> void;
        static auto passDependsOn(const RenderPassNode& A, const RenderPassNode& B) -> bool ;
        auto inferOutputs() -> void;
        auto topologicalSort() const -> vot::vector<RenderPassNode>;
        auto topologicalSortWithLayer() const -> vot::vector<RenderPassNode>;
        auto drawFlowNodeGraph() const -> void;

        vot::vector<RenderPassNode> passes;
        vot::vector<RenderPassNode> sorted;
        vot::vector<RT_sptr> images;
        vot::RHandle RHandle{};
    };

    namespace Node {
        struct read{
            vot::vector<RT_sptr> images;

            explicit read(const vot::vector<vot::Image_sptr>& imgs) {
                for (auto& img : imgs) {
                    images.emplace_back(std::make_shared<RenderTarget>(img));
                }
            }

            template<typename ...Args>
            requires(sizeof...(Args)>0 && (std::conjunction_v<std::is_same<std::decay_t<Args>, vot::Image_sptr>...>))
            explicit read(Args&&...imgs) {
                (images.emplace_back(std::make_shared<RenderTarget>(imgs)), ...);
            }
        };
        struct write {
            vot::vector<RT_sptr> images;

            template<typename ...Args>
            explicit write(Args&&...imgs) {
                (images.emplace_back(std::make_shared<RenderTarget>(imgs)), ...);
            }
        };
        struct invoke{ std::function<void(vot::CommandBuffer&)> execute; };
    }

    struct PassNode {
        vot::vector<std::function<void(RenderPassNode&)>> passes;

        PassNode() = default;
        explicit PassNode(const RT_sptr &target){
            passes.emplace_back([=](RenderPassNode& node){ node.target = target; });
        }
        explicit PassNode(const vot::Image_sptr &target){
            passes.emplace_back([=](RenderPassNode& node){ node.target = std::make_shared<RenderTarget>(target); });
        }
        explicit PassNode(const vot::string& n) {
            passes.emplace_back([=](RenderPassNode& node) { node.name = n; });
        }
    };

    inline PassNode operator+(PassNode A, const Node::read &B) {
        A.passes.emplace_back([&](RenderPassNode& p) {
            for (auto& img : B.images) {
                p.inputs.emplace_back(img);
            }
        });
        return A;
    }
    inline PassNode operator+(PassNode A, const Node::write &B) {
        A.passes.emplace_back([&](RenderPassNode& p) {
            for (auto& img : B.images) {
                p.outputs.emplace_back(img);
            }
        });
        return A;
    }
    inline PassNode operator>>(PassNode A, const Node::invoke &B) {
        A.passes.emplace_back([=](RenderPassNode& p) { p.execute = B.execute; });
        return A;
    }


    class RG_DSL {
        struct RG_Begin_t{};
        struct RG_End_t{};
    public:
        static constexpr RG_Begin_t begin{};
        static constexpr RG_End_t end{};

        template<typename T>
        static auto ctx(T &&ptr) -> RenderGraph * {
            if constexpr (std::is_pointer_v<std::decay_t<T> >) {
                graph = ptr;
            } else if constexpr (requires { ptr.get(); }) {
                graph = ptr.get();
            } else {
                static_assert(false, "ctx() requires pointer or smart pointer to RenderGraph");
            }
            return graph;
        }

        ~RG_DSL() = default;

        auto operator|(const PassNode& node)&& -> RG_DSL {
            passes.emplace_back(node);
            return *this;
        }

        friend auto operator|(RenderGraph* g, RG_Begin_t) -> RG_DSL {
            if (!g) throw std::runtime_error("RenderGraph context not set!");
            g->begin();
            RG_DSL dsl{};
            return dsl;
        }

        friend auto operator|(RG_DSL&& dsl, std::nullptr_t) -> RG_DSL {
            if (!graph) throw std::runtime_error("RenderGraph context not set!");
            for (auto &n : dsl.passes) {
                RenderPassNode pass{};
                for (auto &f : n.passes) f(pass);
                graph->addPass(pass);
            }
            if (graph) graph->end();

            return dsl;
        }
        friend auto operator|(RG_DSL&& dsl, RG_End_t) -> RG_DSL {
            if (!graph) throw std::runtime_error("RenderGraph context not set!");
            for (auto &n : dsl.passes) {
                RenderPassNode pass{};
                for (auto &f : n.passes) f(pass);
                graph->addPass(pass);
            }
            if (graph) graph->end();
            return dsl;
        }

        struct Lambda {
            std::function<void(RG_DSL&)> fn;
        };

        template<typename F>
        static Lambda lambda(F&& f) {
           // return Build{std::function<void(RG_DSL&)>(std::forward<F>(f))};
            return Lambda{std::function<void(RG_DSL&)>(std::forward<F>(f))};
        }

        // template<typename F>
        // friend auto operator|(RG_DSL &&dsl, F &&f)
        //     -> std::enable_if<std::is_invocable_v<F, RG_DSL &>, RG_DSL> {
        //     f(dsl);
        //     return std::move(dsl);
        // }
        friend auto operator|(RG_DSL &&dsl, const Lambda &b) -> RG_DSL {
            b.fn(dsl);
            return std::move(dsl);
        }

    private:
        inline static RenderGraph* graph = nullptr;
        vot::vector<PassNode> passes;
    };


} // sc

#endif //VKCELSHADINGRENDERER_RENDERGRAPH_H
