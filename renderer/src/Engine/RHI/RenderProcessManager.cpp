//
// Created by lenovo on 7/17/2024.
//

#include "RenderProcessManager.h"

namespace yic {

    RenderProcessManager::RenderProcessManager() {
        mRenderProcess[enum_name(RenderProcessPhases::ePrimary)] = std::make_shared<RenderProcess>(enum_name(RenderProcessPhases::ePrimary));

        mRenderGroup = RenderGroup::configure(FrameRender::eColorDepthStencilRenderPass)
                ->addDesSetLayout_1(0, 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
                ->addPushConstantRange_2(vk::ShaderStageFlagBits::eVertex, 0, sizeof (glm::mat4))
                ->addBindingDescription_3(0, sizeof(sc::Vertex), vk::VertexInputRate::eVertex)
                ->addAttributeDescription_4(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(sc::Vertex, pos))
                ->addAttributeDescription_4(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(sc::Vertex, nor))
                ->addAttributeDescription_4(2, 0, vk::Format::eR32G32Sfloat, offsetof(sc::Vertex, uv))
                ->addShader_5("v_test.spv", vk::ShaderStageFlagBits::eVertex)
                ->addShader_5("f_test.spv", vk::ShaderStageFlagBits::eFragment)
        //        ->addModel(R"(E:\Material\model\Nilou\Nilou.pmx)")
                ->build();

        yic::EventBus::subscribeAuto([&](const et::modelPath& modelPaths){
            for(auto& pt : modelPaths.paths_ref()){
                mRenderGroup->addModel(pt);
            }
        });
    }

    RenderProcessManager::~RenderProcessManager() = default;

    auto RenderProcessManager::RenderProcedure() -> std::vector<vk::CommandBuffer> {
        auto& cmds = get()->cmds;

        cmds.clear();

        for(auto& key : {RenderProcessPhases::ePrimary}){
            auto it = get()->mRenderProcess.find(enum_name(key));
            if (it != get()->mRenderProcess.end()){
                if (it->second->process().has_value())
                    cmds.emplace_back(it->second->process().value());
            }
        }

        return {cmds.begin(), cmds.end()};
    }

    auto RenderProcessManager::prepare() -> void {
        sc::globalCamera.computeViewProjMatrix();

        tbb::parallel_for_each(get()->mRenderProcess.begin(), get()->mRenderProcess.end(), [](auto& it){
            if (it.second){
                it.second->prepare();
            }
        });

        get()->mRenderProcess[enum_name(RenderProcessPhases::ePrimary)]->appendCommandRecord([&](vk::CommandBuffer &cmd) {
            get()->mRenderGroup->render(cmd)
                    .bindPipeline()
                    .pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4),
                                   sc::globalCamera.getVpMatrix())
                    .drawModel();
        });

    }

} // yic