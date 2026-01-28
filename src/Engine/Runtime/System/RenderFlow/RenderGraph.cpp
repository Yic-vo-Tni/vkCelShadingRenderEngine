//
// Created by lenovo on 7/22/2025.
//

#include "RenderGraph.h"

#include "Editor/ImGuiHub.h"
#include "RS/ResourceSystem.h"
#include "RHI/Command.h"
#include "RHI/GpuRuntime/CommandCollector.h"

namespace runtime::flow {
    RenderGraph::RenderGraph() {
        RHandle = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);
    }

    auto RenderGraph::addPass(const RenderPassNode &pass) -> void {
        passes.emplace_back(pass);
    }

    auto RenderGraph::end() -> void {
        compile();
        drawFlowNodeGraph();
    }

    auto RenderGraph::compile() -> void {
        // if (sorted.empty()) {
        //     inferOutputs();
        //     sorted = topologicalSortWithLayer(); //FIXME: temp
        // }
        sorted.clear();
        inferOutputs();
        sorted = topologicalSortWithLayer(); // HACK: temporary, and need eventhub to control when to restart

        // tbb::parallel_for_each(sorted, [&](auto &pass) {
        //     yic::command2->bind(pass.order, sorted.size(),[&](vot::CommandBuffer &cmd) {
        //         if (pass.target) {
        //             if (pass.execute) {
        //                 pass.target->draw(cmd, [&] { pass.execute(cmd); });
        //             } else {
        //                 pass.target->draw(cmd, [&]{});
        //             }
        //
        //         }
        //         else if (pass.execute)
        //             pass.execute(cmd);
        //     });
        // });

        for (auto &pass : sorted) {
            yic::command2->bind(pass.order, sorted.size(), [&](vot::CommandBuffer &cmd) {
                if (pass.target) {
                    if (pass.execute) {
                        pass.target->draw(cmd, [&] { pass.execute(cmd); });
                    } else {
                        pass.target->draw(cmd, [&] {
                        });
                    }
                } else if (pass.execute)
                    pass.execute(cmd);
            });
        } // temp:

        yic::command2->submit(); // CRITICAL: hard-coded logic, causes when sorted is empty
    }

    auto RenderGraph::passDependsOn(const RenderPassNode &A, const RenderPassNode &B) -> bool {
        for (const auto& in : A.inputs) {
            if (!in) continue;
            for (const auto& out : B.outputs) {
                if (!out) continue;
                if (in == out) return true;
                if (in->va() && out->va() && in->va()->id == out->va()->id)
                    return true;
            }
        }
        return false;
    }

    auto RenderGraph::inferOutputs() -> void {
        vot::unordered_map<vot::string, size_t> lastProducer;

        for (size_t i = 0; i < passes.size(); ++i) {
            if (passes[i].target)
                lastProducer[passes[i].target->va()->id] = i;
        }

        for (size_t i = 0; i < passes.size(); ++i) {
            for (auto& out : passes[i].outputs) {
                if (out) {
                    lastProducer[out->va()->id] = i;
                }
            }
        }

        for (size_t i = 0; i < passes.size(); ++i) {
            for (auto& in : passes[i].inputs) {
                if (!in) continue;
                auto id = in->va()->id;

                if (!lastProducer.contains(id)) {
                    lastProducer[id] = i;
                    continue;
                }

                size_t producerIdx = lastProducer[id];
                auto& producer = passes[producerIdx];

                bool exists = std::ranges::any_of(
                    producer.outputs,
                    [&](auto& o) { return o && o->va()->id == id; });

                if (!exists)
                    producer.outputs.push_back(in);
            }
        }

        for (size_t i = 0; i < passes.size(); ++i) {
            for (auto& out : passes[i].outputs)
                if (out)
                    lastProducer[out->va()->id] = i;
        }
    }

    auto RenderGraph::topologicalSort() const -> vot::vector<RenderPassNode> {
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

        vot::vector<RenderPassNode> sorted_;
        while (!q.empty()) {
            int idx = q.front(); q.pop();
            sorted_.push_back(passes[idx]);
            for (int to : graph[idx]) {
                if (--indegree[to] == 0) q.push(to);
            }
        }

        if (sorted_.size() != passes.size())
            throw std::runtime_error("RenderGraph Pass has circular dependency!");
        return sorted_;
    }

    auto RenderGraph::topologicalSortWithLayer() const -> vot::vector<RenderPassNode> {
        const size_t N = passes.size();
        vot::unordered_map<int, vot::unordered_set<int>> graph;
        vot::vector<int> indegree(N, 0);
        vot::vector<int> layer(N, 0);

        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                if (i == j) continue;
                if (passDependsOn(passes[i], passes[j])) {
                    graph[j].insert(i);
                    indegree[i]++;
                }
            }
        }

        vot::queue<int> q;
        for (size_t i = 0; i < N; ++i)
            if (indegree[i] == 0)
                q.push(static_cast<int>(i));

        vot::vector<RenderPassNode> sorted_;
        sorted_.reserve(N);
        std::uint32_t order = 0;

        while (!q.empty()) {
            int idx = q.front(); q.pop();

            RenderPassNode node = passes[idx];
            node.order = order++;
            node.layer = layer[idx];
            sorted_.push_back(std::move(node));

            for (int to : graph[idx]) {
                layer[to] = std::max(layer[to], layer[idx] + 1);
                if (--indegree[to] == 0)
                    q.push(to);
            }
        }

        if (sorted_.size() != passes.size())
            throw std::runtime_error("RenderGraph Pass has circular dependency!");

        return sorted_;
    }

    auto RenderGraph::drawFlowNodeGraph() const -> void {
        yic::imguiHub->to(vot::uiWidget::eNodeWidget, [&] {
            // ImNodes::BeginNodeEditor();
            // constexpr float spacingX = 250.0f;
            // constexpr float spacingY = 180.0f;
            // constexpr auto originOffset = ImVec2(100.0f, 80.0f);
            //
            // static float zoom = 1.0f;
            // const ImGuiIO &io = ImGui::GetIO();
            // if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
            //     zoom = std::clamp(zoom + io.MouseWheel * 0.1f, 0.25f, 2.0f);
            // }
            // ImGui::SetWindowFontScale(zoom);
            //
            // struct Edge {
            //     int from;
            //     int to;
            // };
            // std::vector<Edge> edges_all;
            // const size_t N = sorted.size();
            //
            // for (size_t i = 0; i < N; ++i)
            //     for (size_t j = 0; j < N; ++j)
            //         if (i != j && passDependsOn(sorted[i], sorted[j]))
            //             edges_all.push_back({static_cast<int>(j), static_cast<int>(i)});
            //
            // auto hasIndirectPath = [&](int a, int b) {
            //     std::queue<int> q;
            //     std::unordered_set<int> visited;
            //     q.push(a);
            //     visited.insert(a);
            //     while (!q.empty()) {
            //         int cur = q.front();
            //         q.pop();
            //         for (auto &e: edges_all) {
            //             if (e.from == cur && e.to != b && !visited.contains(e.to)) {
            //                 if (e.to == b) return true;
            //                 q.push(e.to);
            //                 visited.insert(e.to);
            //             }
            //         }
            //     }
            //     return false;
            // };
            //
            // std::vector<Edge> edges;
            // edges.reserve(edges_all.size());
            // for (auto &e: edges_all) {
            //     bool redundant = false;
            //     for (auto &other: edges_all) {
            //         if (other.from == e.from && other.to != e.to) {
            //             if (hasIndirectPath(other.to, e.to)) {
            //                 redundant = true;
            //                 break;
            //             }
            //         }
            //     }
            //     if (!redundant)
            //         edges.push_back(e);
            // }
            //
            // static std::unordered_set<int> positionedNodes;
            // std::unordered_map<int, int> layerCount;
            // for (size_t i = 0; i < N; ++i) {
            //     const auto &node = sorted[i];
            //     const int nodeId = static_cast<int>(i);
            //     if (!positionedNodes.contains(nodeId)) {
            //         int layer = static_cast<int>(node.layer);
            //         int row = layerCount[layer]++;
            //         ImVec2 pos{
            //             originOffset.x + layer * spacingX,
            //             originOffset.y + row * spacingY
            //         };
            //         ImNodes::SetNodeEditorSpacePos(nodeId, pos);
            //         positionedNodes.insert(nodeId);
            //     }
            // }
            //
            // for (size_t i = 0; i < N; ++i) {
            //     const auto &pass = sorted[i];
            //     const int nodeId = static_cast<int>(i);
            //     ImNodes::BeginNode(nodeId);
            //
            //     ImNodes::BeginNodeTitleBar();
            //     ImGui::Text("%s", pass.target ? pass.target->va()->id.c_str() : pass.name.c_str());
            //     ImNodes::EndNodeTitleBar();
            //
            //     ImGui::Dummy(ImVec2(150.f, 0.0f));
            //
            //     ImNodes::BeginInputAttribute(nodeId * 10 + 1);
            //     ImGui::Text("Inputs: %d", (int) pass.inputs.size());
            //     ImNodes::EndInputAttribute();
            //
            //     ImNodes::BeginOutputAttribute(nodeId * 10 + 2);
            //     ImGui::Text("Outputs: %d", (int) pass.outputs.size());
            //     ImNodes::EndOutputAttribute();
            //
            //     ImNodes::EndNode();
            // }
            //
            // int link_id = 0;
            // for (auto &[from, to]: edges) {
            //     const int outAttr = from * 10 + 2;
            //     const int inAttr = to * 10 + 1;
            //     ImNodes::Link(link_id++, outAttr, inAttr);
            // }
            //
            // ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
            // ImNodes::EndNodeEditor();
            // ImGui::SetWindowFontScale(1.0f);
                //
                // ImNodes::BeginNodeEditor();
                // ImNodes::EndNodeEditor();
        });
    }
} // sc