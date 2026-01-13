//
// Created by lenovo on 12/29/2025.
//

#include "DynamicEditableRendering.h"

#include "Editor/ImGuiHub.h"
#include "RHI/Allocator.h"
#include "Runtime/System/RenderStage/RenderStage.h"
#include "Runtime/System/RenderStage/AlgorithmNode/Forward.h"

namespace runtime::flow {

    DynamicEditableRendering::DynamicEditableRendering() = default;

    DynamicEditableRendering::~DynamicEditableRendering() = default;


    auto DynamicEditableRendering::drawEditor() -> void {
        auto makeAttrId = [](const std::uint32_t& nodeId, const int port){
            return (1 << 30) | nodeId << 8 | port;
        };

        yic::imguiHub->to(vot::uiWidget::eDERWidget, [&] {
            ImNodes::BeginNodeEditor();

            for (auto &[name, nodes]: rf.algorithms) {
                for (auto &node: nodes) {
                    ImNodes::BeginNode(node.id);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("%s", node.name.c_str());
                    ImNodes::EndNodeTitleBar();

                    ImNodes::BeginInputAttribute(makeAttrId(node.id, 0));
                    ImGui::Text("Input");
                    ImNodes::EndInputAttribute();

                    ImNodes::BeginOutputAttribute(makeAttrId(node.id, 1));
                    ImGui::Text("Output");
                    ImNodes::EndOutputAttribute();

                    ImNodes::EndNode();
                }
            }

            ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
            ImNodes::EndNodeEditor();
        });


        yic::imguiHub->to(vot::uiWidget::ePanelWidget, [&] {
            yic::imguiHub->collapsingHeader("DER Node Inspector", [&] {
                const DER::AlgorithmNode *selectedNode = nullptr;
                for (auto &[name, nodes]: rf.algorithms) {
                    for (auto &node: nodes) {
                        if (ImNodes::IsNodeSelected(node.id)) {
                            selectedNode = &node;
                            break;
                        }
                    }
                }
                if (!selectedNode) {
                    ImGui::Text("No Selected Node");
                    return;
                }

                ImGui::Text("Node ID: %u", selectedNode->id);
            });


            yic::imguiHub->collapsingHeader("DER Control", [&] {
                static char rtName[64] = "RT::NewRTName";
                ImGui::InputText("RT Name", rtName, IM_ARRAYSIZE(rtName));

                yic::imguiHub->button("Add RT", [&] {
                    rf.algorithms.emplace_back(
                        DER::flow | [&](DER::AlgorithmFlow &flow) {
                            flow.name = rtName;
                            flow.nodes.emplace_back(
                                DER::node | [&](DER::AlgorithmNode &node) {
                                    node.id = ++nodeId;
                                    assert(nodeId < (1u << 23));
                                    node.name = rtName;
                                    node.buildTarget = [&] {
                                        return yic::allocator->allocImage(vot::ImageCI()
                                            .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                                            .updateColorToImGui(vot::uiWidget::eRenderWidget)
                                            .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                                            .setFormat(vk::Format::eR16G16B16A16Sfloat)
                                            .setImageCount(3)
                                            .setExtent(vot::Resolutions::eQHDExtent)
                                            .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), rtName);
                                    };
                                    node.THandle = node.buildTarget();
                                }
                            );
                        });
                });
            });

            yic::imguiHub->collapsingHeader("Temp Test", [&] {
                yic::imguiHub->button("Add Compose", [&] {
                    // rf.algorithms.emplace_back(
                    //     DER::flow | [&](DER::AlgorithmFlow &flow) {
                    //         flow.name = "Compose";
                    //         flow.nodes.emplace_back(
                    //             DER::node | [&](DER::AlgorithmNode &node) {
                    //                 node.id = ++nodeId;
                    //                 assert(nodeId < (1u << 23));
                    //                 node.name = "Compose";
                    //                 node.buildTarget = Compose_Target;
                    //                 node.buildPipeline = Compose_Pipline;
                    //                 node.dispatch = Compose_Dispatch;
                    //                 node.THandle = node.buildTarget();
                    //                 node.PHandle = node.buildPipeline();
                    //             });
                    //
                    //     });
                    using PFN_Compose_Target = vot::ImageCI(*)();
                    using PFN_Compose_Pipeline = vot::PipelineLibrary(*)();
                    using PFN_Compose_Dispatch = void(*)(vot::CommandBuffer &, const DERTranslator*);
                    struct ComposeDll {
                        HMODULE dll = nullptr;
                        PFN_Compose_Target buildTarget = nullptr;
                        PFN_Compose_Pipeline buildPipeline = nullptr;
                        PFN_Compose_Dispatch dispatch = nullptr;
                    };

                    static ComposeDll composeDll;

                    auto loadComposeDll = [&] -> bool {
                        composeDll.dll = ::LoadLibraryA("libComposed.dll");
                        if (!composeDll.dll) { return false; }

                        composeDll.buildTarget = reinterpret_cast<PFN_Compose_Target>(::GetProcAddress(
                            composeDll.dll, "Compose_Target"));
                        composeDll.buildPipeline = reinterpret_cast<PFN_Compose_Pipeline>(::GetProcAddress(
                            composeDll.dll, "Compose_Pipeline"));
                        composeDll.dispatch = reinterpret_cast<PFN_Compose_Dispatch>(::GetProcAddress(
                            composeDll.dll, "Compose_Dispatch"));

                        return composeDll.buildTarget && composeDll.buildPipeline && composeDll.dispatch;
                    };

                    static bool loaded = false;
                    if (!loaded) {
                        loaded = loadComposeDll();
                        assert(loaded && "Failed to load Compose.dll");
                    }

                    // const std::function buildT = composeDll.buildTarget;
                    // auto t = yic::allocator->allocImage(buildT(), "Compose");
                    // const std::function<PipelineHandle()> buildPipelineFn = composeDll.buildPipeline;
                    // auto pipeline = buildPipelineFn();


                    rf.algorithms.emplace_back(
                        DER::flow | [&](DER::AlgorithmFlow &flow) {
                            flow.name = "Compose";
                            flow.nodes.emplace_back(
                                DER::node | [&](DER::AlgorithmNode &node) {
                                    node.id = ++nodeId;
                                    assert(nodeId < (1u << 23));
                                    node.name = "Compose";
                                    // node.buildTarget = composeDll.buildTarget;
                                    // node.buildPipeline = composeDll.buildPipeline;
                                    // node.dispatch = composeDll.dispatch;
                                    // node.THandle = node.buildTarget();
                                    // node.PHandle = node.buildPipeline();
                                    node.buildTarget = []{ return yic::allocator->allocImage(composeDll.buildTarget(), "Compose");};
                                    node.THandle = node.buildTarget();
                                    node.buildPipeline = [] {
                                        auto pipeline = std::make_shared<rhi::GraphicsPipeline>();
                                        pipeline->combinePipelineLibrary(composeDll.buildPipeline());
                                        return pipeline;
                                    };
                                    node.PHandle = node.buildPipeline();
                                    node.dispatch = [&](vot::CommandBuffer& cmd){ composeDll.dispatch(cmd, yic::derTranslator);};
                                });
                        });

                    //
                });
            });
        });
    }
} // flow