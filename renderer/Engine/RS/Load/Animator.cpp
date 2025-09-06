//
// Created by lenovo on 6/12/2025.
//

#include "Animator.h"
#include "Core/Management/TripleBufferIndexManager.h"

#include <saba/src/Saba/Base/Path.h>
#include <saba/src/Saba/Model/MMD/PMDModel.h>
#include <saba/src/Saba/Model/MMD/VMDFile.h>
#include <saba/src/Saba/Model/MMD/VMDAnimation.h>
#include <saba/src/Saba/Model/MMD/VMDCameraAnimation.h>

namespace rs {

    auto Animator::sampleAnimation(float deltaTime, vot::AnimationComponent& ac) -> void {
        const auto sIndex = yic::indexRing.get(vot::LogicBufferType::eSlow).logic_cur();
        const auto& anim = ac.animations[ac.activeAnim].second;
        auto& boneMats = ac.boneMats[sIndex];
        //auto& boneMats = ac.boneMats;
        auto& boneMatBuf = ac.boneMatBuffer;

        float tps = anim->getTicksPerSecond();
        if (tps < 1e-3f) {
            tps = 25.f;
        } else if (tps > 120.f) {
            //tps /= 10.f;
        }
        mAnimTime += tps * deltaTime;

        mAnimTime = fmod(mAnimTime, anim->getDuration());

        std::function<void(const vot::BoneNode* node, glm::mat4 parentTransform)> calculateBoneTransform = [&](const vot::BoneNode* node, glm::mat4 parentTransform) -> void{
            const auto nodeName = node->name;
            auto nodeTransform = node->transformation;

            if (const auto bone = anim->findBone(nodeName)){
                bone->Update(mAnimTime);
                nodeTransform = bone->GetLocalTransform();
            }

            const auto globalTransform = parentTransform * nodeTransform;
            auto boneInfoMap = ac.boneMap;
            if (boneInfoMap.contains(nodeName)){
                const auto index = boneInfoMap[nodeName].id;
                const auto invBind = boneInfoMap[nodeName].offset;
                boneMats[index] = globalTransform * invBind;
            }

            for(auto i = 0u; i < node->childrenCount; i++){
                calculateBoneTransform(&node->children[i], globalTransform);
            }
        };

        calculateBoneTransform(&ac.boneNode, glm::mat4 (1.f));
        // if (boneMatBuf){
        //     boneMatBuf->update(boneMats);
        // };
    }


    auto Animator::bindVmd(const std::pair<vot::string, saba::VMDFile>& vmdFile, const vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void {
        auto vmd = std::make_unique<saba::VMDAnimation>();
        if (!vmd->Create(vc.pmx)){
            yic::logger->error("failed to load vmd");
        }

        if (!vmd->Add(vmdFile.second)){

        }
        vmd->SyncPhysics(0.f);

        ac.vmd = std::pair(vmdFile.first, std::move(vmd));
    }

    auto Animator::sampleVmd(vot::VertexDataComponent& vc, const vot::RenderComponent& rc) -> void {
        vc.pmx->Update();
        const auto pos = vc.pmx->GetUpdatePositions();
        const auto nor = vc.pmx->GetUpdateNormals();
        const auto uv = vc.pmx->GetUpdateUVs();

        const auto index = yic::indexRing.get(vot::LogicBufferType::eSlow).logic_cur();

        for (auto i = 0; i < vc.pmx->GetVertexCount(); i++) {
            vc.mmd_vertices_pmr[vot::VertexDataComponent::eAnim][i] = vot::VertexT<vot::eMMD>{pos[i], nor[i], uv[i]};
        }

        rc.vertexBuffer[index]->update(vc.mmd_vertices_pmr[vot::VertexDataComponent::eAnim]);
    }

} // rs