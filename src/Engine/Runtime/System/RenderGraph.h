//
// Created by lenovo on 7/22/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERGRAPH_H
#define VKCELSHADINGRENDERER_RENDERGRAPH_H

#include "Runtime/System/RenderFlow/RenderTarget.h"

namespace sc {

    struct RenderPassNode{
        runtime::flow::RT_sptr target;
        vot::vector<runtime::flow::RT_sptr> inputs;
        vot::vector<runtime::flow::RT_sptr> outputs;
        std::optional<vot::ImageDrawCI> drawci{std::nullopt};
        std::function<void(vot::CommandBuffer&)> execute;
    };

    class RenderGraph {
    public:
        RenderGraph();
        auto addPass(const RenderPassNode& pass) -> void;

        auto begin() -> void{ passes.clear(); }
        auto end() -> void;
    private:
        static auto passDependsOn(const RenderPassNode& A, const RenderPassNode& B) -> bool ;
        auto topologicalSort() -> vot::vector<RenderPassNode>;

        vot::vector<RenderPassNode> passes;
        vot::vector<runtime::flow::RT_sptr> images;
        vot::RHandle RHandle{};
    };

    namespace Node {
        struct in{
            runtime::flow::RT_sptr image;
            explicit in(const vot::Image_sptr& img): image(std::make_shared<runtime::flow::RenderTarget>(img)) {}
        };
        struct out{ runtime::flow::RT_sptr image; };
        struct ctx{ vot::ImageDrawCI drawci; };
        struct exec{ std::function<void(vot::CommandBuffer&)> execute; };
    }

    struct PassNode {
        vot::vector<std::function<void(RenderPassNode&)>> passes;

        PassNode() = default;
        explicit PassNode(const runtime::flow::RT_sptr &target){
            passes.emplace_back([=](RenderPassNode& node){ node.target = target; });
        }
        explicit PassNode(const vot::Image_sptr &target){
            passes.emplace_back([=](RenderPassNode& node){ node.target = std::make_shared<runtime::flow::RenderTarget>(target); });
        }
    };

    inline PassNode operator+(PassNode A, const Node::in &B) {
        A.passes.emplace_back([=](RenderPassNode& p) { p.inputs.emplace_back(B.image); });
        return A;
    }
    inline PassNode operator+(PassNode A, const Node::out &B) {
        A.passes.emplace_back([=](RenderPassNode& p) { p.outputs.emplace_back(B.image); });
        return A;
    }
    inline PassNode operator+(PassNode A, const Node::ctx &B) {
        A.passes.emplace_back([=](RenderPassNode& p) { p.drawci = B.drawci; });
        return A;
    }
    inline PassNode operator>>(PassNode A, const Node::exec &B) {
        A.passes.emplace_back([=](RenderPassNode& p) { p.execute = B.execute; });
        return A;
    }


    class RG_DSL {
        struct RG_Begin_t{};
        struct RG_End_t{};
    public:
        inline static constexpr RG_Begin_t begin{};
        inline static constexpr RG_End_t end{};
    public:
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

    private:
        inline static RenderGraph* graph = nullptr;
        vot::vector<PassNode> passes;
    };


} // sc

#endif //VKCELSHADINGRENDERER_RENDERGRAPH_H
