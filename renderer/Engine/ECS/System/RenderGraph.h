//
// Created by lenovo on 7/22/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERGRAPH_H
#define VKCELSHADINGRENDERER_RENDERGRAPH_H

namespace sc {

    struct RenderPassNode{
        vot::Image_sptr target;
        vot::vector<vot::Image_sptr> inputs;
        vot::vector<vot::Image_sptr> outputs;
        std::optional<vot::ImageDrawCI> drawci{std::nullopt};
        std::function<void()> execute;
    };

    class RenderGraph {
    public:
        auto addPass(const RenderPassNode& pass) -> void;

        auto clear() -> void{ passes.clear(); }
        auto exe(vot::CommandBuffer& cmd) -> void;
    private:
        auto passDependsOn(const RenderPassNode& A, const RenderPassNode& B) -> bool ;
        auto topologicalSort() -> vot::vector<RenderPassNode>;

        vot::vector<RenderPassNode> passes;
        vot::vector<vot::Image_sptr> images;
    };

} // sc

#endif //VKCELSHADINGRENDERER_RENDERGRAPH_H
