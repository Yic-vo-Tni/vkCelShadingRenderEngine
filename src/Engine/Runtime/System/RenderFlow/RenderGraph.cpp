//
// Created by lenovo on 7/22/2025.
//

#include "RenderGraph.h"

#include "Editor/ImGuiHub.h"
#include "RS/ResourceSystem.h"
#include "RHI/Command.h"
#include "RHI/RHI2/CommandCollector.h"

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
        // yic::command->bind(vot::SubmitInfo()
        //                    .setRHandle(RHandle)
        //                    .setQueueType(vot::queueType::eUndefined)
        //                    .setWaitValues(vot::timelineStage::ePrepare)
        //                    .setSignalValues(vot::timelineStage::eFinish)
        //                    .setWaitStageMasks(vk::PipelineStageFlagBits::eTopOfPipe), [&](vot::CommandBuffer &cmd) {
        //                        if (sorted.empty()) sorted = topologicalSort(); // FIXME: temp
        //                        for (auto &pass: sorted) {
        //                            if (pass.target) {
        //                                pass.target->draw(cmd, [&] { pass.execute(cmd); });
        //                            } else {
        //                                if (pass.execute) {
        //                                    pass.execute(cmd);
        //                                }
        //                            }
        //                        }
        //                    });

        // yic::command2->bind([&](vot::CommandBuffer &cmd) {
        //     if (sorted.empty()) sorted = topologicalSort(); // FIXME: temp
        //     for (auto &pass: sorted) {
        //         if (pass.target) {
        //             pass.target->draw(cmd, [&] { pass.execute(cmd); });
        //         } else {
        //             if (pass.execute) {
        //                 pass.execute(cmd);
        //             }
        //         }
        //     }
        // });

        if (sorted.empty()) {
            inferOutputs();
            sorted = topologicalSortWithLayer();
        }
        //if (sorted.empty()) sorted = topologicalSort();

        // for (auto& [] : sorted) {
        //     fmt::print("  [{}] {} -> ", src, passes[src].name);
        //     for (auto dst : dsts)
        //         fmt::print("{} ", passes[dst].name);
        //     fmt::print("\n");
        // }
        // fmt::print("\n========== [RenderGraph: Pass Inputs / Outputs] ==========\n");
        // for (size_t i = 0; i < passes.size(); ++i) {
        //     const auto& p = passes[i];
        //     fmt::print("Pass[{}]: {}\n", i, p.name);
        //
        //     if (p.inputs.empty() && p.outputs.empty() && !p.target) {
        //         fmt::print("  (no inputs/outputs)\n");
        //         continue;
        //     }
        //
        //     if (p.target)
        //         fmt::print("  target -> {}\n", p.target->va()->id);
        //
        //     for (auto& in : p.inputs)
        //         fmt::print("  in  -> {}\n", (in && in->va()) ? in->va()->id : "null");
        //
        //     for (auto& out : p.outputs)
        //         fmt::print("  out -> {}\n", (out && out->va()) ? out->va()->id : "null");
        // }
        // for (auto &pass: sorted) {
        //     // yic::command2->bind([&](vot::CommandBuffer &cmd) {
        //     //     if (pass.target) {
        //     //         pass.target->draw(cmd, [&] { pass.execute(cmd); });
        //     //     } else {
        //     //         if (pass.execute) {
        //     //             pass.execute(cmd);
        //     //         }
        //     //     }
        //     // });
        //
        //     yic::logger->warn("order:{0}, layer:{1}", pass.order, pass.layer);
        //
        //     yic::command2->bind(pass.order, [&](vot::CommandBuffer &cmd) {
        //         if (pass.target) {
        //             pass.target->draw(cmd, [&] { pass.execute(cmd); });
        //         } else {
        //             if (pass.execute) {
        //                 pass.execute(cmd);
        //             }
        //         }
        //     });
        // }

        tbb::parallel_for_each(sorted, [&](auto &pass) {
            yic::command2->bind(pass.order, sorted.size(),[&](vot::CommandBuffer &cmd) {
                if (pass.target)
                    pass.target->draw(cmd, [&] { pass.execute(cmd); });
                else if (pass.execute)
                    pass.execute(cmd);
            });
        });

        yic::command2->submit();
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
                q.push(int(i));

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
            ImNodes::BeginNodeEditor();
            constexpr float spacingX = 200.0f;
            constexpr float spacingY = 250.0f;
            constexpr int nodesPerRow = 5;
            constexpr auto originOffset = ImVec2(150.0f, 100.0f);
            static std::unordered_set<int> positionedNodes;

            static float zoom = 1.0f;
            const ImGuiIO &io = ImGui::GetIO();

            if (ImGui::IsWindowHovered()) {
                if (io.MouseWheel != 0.0f) {
                    zoom += io.MouseWheel * 0.1f;
                    zoom = std::clamp(zoom, 0.2f, 2.5f);
                }
            }

            ImGui::SetWindowFontScale(zoom);

            for (size_t i = 0; i < sorted.size(); ++i) {
                const auto& pass = sorted[i];

                if (!positionedNodes.contains(static_cast<int>(i))) {
                    const int row = static_cast<int>(i) / nodesPerRow;
                    const int col = static_cast<int>(i) % nodesPerRow;

                    const float posX = originOffset.x + col * spacingX;
                    const float posY = originOffset.y + row * spacingY;

                    ImNodes::SetNodeEditorSpacePos(static_cast<int>(i), ImVec2(posX, posY));
                    positionedNodes.insert(static_cast<int>(i));
                }

                ImNodes::BeginNode(static_cast<int>(i));
                ImNodes::BeginNodeTitleBar();
                ImGui::Text("%s", pass.target ? pass.target->va()->id.c_str() : pass.name.c_str());
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(150.f, 0.0f));

                ImNodes::BeginInputAttribute(static_cast<int>(i * 100));
                ImGui::Text("In");
                ImNodes::EndInputAttribute();

                ImNodes::BeginOutputAttribute(static_cast<int>(i * 1000));
                ImGui::Text("Out");
                ImNodes::EndOutputAttribute();

                ImNodes::EndNode();
            }

            //HACK 伪连线
            for (size_t i = 0; i + 1 < sorted.size(); ++i) {
                const auto out_attr_id = static_cast<int>(i * 1000);
                const auto in_attr_id  = static_cast<int>((i + 1) * 100);
                ImNodes::Link(static_cast<int>(i * 10000), out_attr_id, in_attr_id);
            }

            ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
            ImNodes::EndNodeEditor();

            ImGui::SetWindowFontScale(1.0f);
        });
    }
} // sc