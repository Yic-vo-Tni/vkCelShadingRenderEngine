//
// Created by lenovo on 12/29/2025.
//

#include "DynamicEditableRendering.h"

#include "Editor/ImGuiHub.h"
#include "RHI/GpuRuntime/Alloctor/Allocator.h"
#include "Runtime/System/RenderStage/RenderStage.h"
#include "Runtime/System/RenderStage/AlgorithmNode/Forward.h"

namespace runtime::flow {

    DynamicEditableRendering::DynamicEditableRendering() {
        // rf.algorithms.reserve(16);
        // rf.algorithms.emplace_back(
        //                 DER::flow | [&](DER::AlgorithmFlow &flow) {
        //                     flow.name = "rtName";
        //                     flow.nodes.emplace_back(
        //                         DER::node | [&](DER::AlgorithmNode &node) {
        //                             node.id = ++nodeId;
        //                             assert(nodeId < (1u << 23));
        //                             node.name = "rtName";
        //                             node.buildTarget = [&] {
        //                                 return yic::allocator->allocImage(vot::ImageCI()
        //                                     .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
        //                                     .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
        //                                     .setFormat(vk::Format::eR16G16B16A16Sfloat)
        //                                     .setImageCount(3)
        //                                     .setExtent(vot::Resolutions::eQHDExtent)
        //                                     .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "rtName");
        //                             };
        //                             //node.THandle = node.buildTarget();
        //                             node.THandle = std::make_shared<RenderTarget>(node.buildTarget());
        //                         }
        //                     );
        //                 });

        /////

        // auto node = DER::AlgorithmNode();
        // node.id = ++nodeId;
        // node.name = "rtName";
        // node.buildTarget = [] {
        //     return yic::allocator->allocImage(vot::ImageCI()
        //                                       .setFlags(
        //                                           vot::imageFlagBits::eDepthStencil |
        //                                           vot::imageFlagBits::eDynamicRender)
        //                                       .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
        //                                       .setFormat(vk::Format::eR16G16B16A16Sfloat)
        //                                       .setImageCount(3)
        //                                       .setExtent(vot::Resolutions::eQHDExtent)
        //                                       .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "rtName");
        // };
        // node.THandle = std::make_shared<RenderTarget>(node.buildTarget());
        //
        // auto flow = DER::AlgorithmFlow();
        // flow.name = "rtName";
        // flow.nodes.emplace_back(node);
        // rf.algorithms.emplace_back(flow);

        // rf.algorithms.clear();

    };

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
                                    node.THandle = std::make_shared<RenderTarget>(node.buildTarget());
                                }
                            );
                        });
                });
            });

            yic::imguiHub->collapsingHeader("Temp Test", [&] {
                yic::imguiHub->button("Add Compose", [&] {

                   static ComposeDll composeDll;

                    auto loadComposeDll = [&] -> bool {
#ifdef NDEBUG
                        composeDll.dll = ::LoadLibraryA("libRenderFlow.dll");
#else
                        composeDll.dll = ::LoadLibraryA("libRenderFlowd.dll");
#endif
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
                        assert(loaded && "Failed to load renderflow.dll");
                    }


                    rf.algorithms.emplace_back(
                        DER::flow | [&](DER::AlgorithmFlow &flow) {
                            flow.name = "Compose";
                            flow.nodes.emplace_back(
                                DER::node | [&](DER::AlgorithmNode &node) {
                                    node.id = ++nodeId;
                                    assert(nodeId < (1u << 23));
                                    node.name = "Compose";
                                   // node.buildTarget = [this]{ return yic::allocator->allocImage(composeDll.buildTarget(), "Compose");};
                                    node.buildTarget = []{ return yic::allocator->allocImage(composeDll.buildTarget(), "Compose");};
                                    node.THandle = std::make_shared<RenderTarget>(node.buildTarget());
                                    //node.buildPipeline = [this] {
                                    node.buildPipeline = [] {
                                        const auto pipelineCI = composeDll.buildPipeline();
                                        auto pipeline = std::make_shared<rhi::GraphicsPipeline>();
                                        if (pipelineCI.graphicsPipelineCI.has_value()) {
                                            pipeline->combinePipelineLibrary(pipelineCI.graphicsPipelineCI.value());
                                        }
                                        return pipeline;
                                    };
                                    node.PHandle = node.buildPipeline();
                                    node.dispatch = [=](vot::CommandBuffer &cmd) {
                                       composeDll.dispatch(cmd, yic::derTranslator);
                                    };
                                });
                        });

                });
            });
        });
    }

    auto DynamicEditableRendering::first() -> void {
        // static bool first = true;
        // if (!first) {
        //     return;
        // }
        // rf.algorithms.emplace_back(
        //                 DER::flow | [&](DER::AlgorithmFlow &flow) {
        //                     flow.name = "rtName";
        //                     flow.nodes.emplace_back(
        //                         DER::node | [&](DER::AlgorithmNode &node) {
        //                             node.id = ++nodeId;
        //                             assert(nodeId < (1u << 23));
        //                             node.name = "rtName";
        //                             node.buildTarget = [&] {
        //                                 return yic::allocator->allocImage(vot::ImageCI()
        //                                     .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
        //                                     .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
        //                                     .setFormat(vk::Format::eR16G16B16A16Sfloat)
        //                                     .setImageCount(3)
        //                                     .setExtent(vot::Resolutions::eQHDExtent)
        //                                     .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "rtName");
        //                             };
        //                             node.THandle = node.buildTarget();
        //                         }
        //                     );
        //                 });
        //
        // rf.algorithms.emplace_back(DER::AlgorithmFlow{});
        // rf.algorithms.clear();
        //
        // first = false;
    }
} // flow