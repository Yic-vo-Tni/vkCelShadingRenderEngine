//
// Created by lenovo on 7/22/2025.
//

#include "RenderGraph.h"

#include "Editor/ImGuiHub.h"
#include "RS/ResourceSystem.h"
#include "RHI/Command.h"

namespace sc {
    RenderGraph::RenderGraph() {
        RHandle = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);
    }

    auto RenderGraph::addPass(const RenderPassNode &pass) -> void {
        passes.emplace_back(pass);
    }

    auto RenderGraph::end() -> void {
        yic::command->bind(vot::SubmitInfo()
                           .setRHandle(RHandle)
                           .setQueueType(vot::queueType::eUndefined)
                           .setWaitValues(vot::timelineStage::ePrepare)
                           .setSignalValues(vot::timelineStage::eFinish)
                           .setWaitStageMasks(vk::PipelineStageFlagBits::eTopOfPipe), [&](vot::CommandBuffer &cmd) {
                               sorted = topologicalSort();
                               for (auto &pass: sorted) {
                                        if (pass.target) {
                                            if (pass.drawci != std::nullopt) {
                                                pass.target->drawRender(cmd, pass.drawci.value(), [&]{ pass.execute(cmd); });
                                            } else {
                                                pass.target->drawRendering(cmd, [&]{pass.execute(cmd); });
                                            }
                                        } else {
                                            if (pass.execute) {
                                                pass.execute(cmd);
                                            }
                                        }
                                    }
                           });

        yic::imguiHub->to(vot::uiWidget::eNodeWidget, [&] {
            ImNodes::BeginNodeEditor();
            constexpr float spacingX = 200.0f;
            constexpr float spacingY = 250.0f;
            constexpr int nodesPerRow = 5;
            constexpr auto originOffset = ImVec2(150.0f, 100.0f);
            static std::unordered_set<int> positionedNodes;
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
        });
    }

    auto RenderGraph::passDependsOn(const RenderPassNode &A, const RenderPassNode &B) -> bool {
        // for (const auto& in : A.inputs)
        //     for (const auto& out : B.outputs)
        //         if (in == out) return true;
        // return false;


        for (const auto& in : A.inputs)
            for (const auto& out : B.outputs) {
                if (!in || !out) continue;
                if (in == out) return true;
                if (in->va()->id == out->va()->id) return true;
            }
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