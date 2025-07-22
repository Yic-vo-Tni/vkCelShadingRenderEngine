//
// Created by lenovo on 7/22/2025.
//

#include "RenderGraph.h"
#include "RS/ResourceSystem.h"

namespace sc {
    auto RenderGraph::addPass(const RenderPassNode &pass) -> void {
        passes.emplace_back(pass);
    }

    auto RenderGraph::exe(vot::CommandBuffer &cmd) -> void {
        auto sorted = topologicalSort();
        for(auto& pass : sorted){
            if (pass.drawci != std::nullopt){
                pass.target->drawRender(cmd, pass.drawci.value(), pass.execute);
            } else {
                pass.target->drawRendering(cmd, pass.execute);
            }
        }
    }

    auto RenderGraph::passDependsOn(const RenderPassNode &A, const RenderPassNode &B) -> bool {
        for (const auto& in : A.inputs)
            for (const auto& out : B.outputs)
                if (in == out) return true;
        return false;
    }

    auto RenderGraph::topologicalSort() -> vot::vector<RenderPassNode> {
        vot::unordered_map<int, vot::unordered_set<int>> graph;
        vot::vector<int> indegree(passes.size(), 0);

        for (size_t i = 0; i < passes.size(); ++i)
            for (size_t j = 0; j < passes.size(); ++j)
                if (i != j && passDependsOn(passes[i], passes[j])) {
                    graph[j].insert(i);
                    indegree[i]++;
                }

        vot::queue<int> q;
        for (size_t i = 0; i < passes.size(); ++i)
            if (indegree[i] == 0) q.push(int(i));

        vot::vector<RenderPassNode> sorted;
        while (!q.empty()) {
            int idx = q.front(); q.pop();
            sorted.push_back(passes[idx]);
            for (int to : graph[idx]) {
                if (--indegree[to] == 0) q.push(to);
            }
        }

        if (sorted.size() != passes.size())
            throw std::runtime_error("RenderGraph Pass has circular dependency!");
        return sorted;
    }
} // sc