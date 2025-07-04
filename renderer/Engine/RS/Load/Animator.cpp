//
// Created by lenovo on 6/12/2025.
//

#include "Animator.h"

namespace rs {

    auto Animator::sampleAnimation(float deltaTime, vot::AnimationComponent& ac) -> void {
        auto& anim = ac.animations[ac.activeAnim].second;
        auto& boneMats = ac.boneMats;
        auto& boneMatBuf = ac.boneMatBuffer;
        mAnimTime += anim->getTicksPerSecond() * deltaTime;
        mAnimTime = fmod(mAnimTime, anim->getDuration());

        std::function<void(const vot::BoneNode* node, glm::mat4 parentTransform)> calculateBoneTransform = [&](const vot::BoneNode* node, glm::mat4 parentTransform) -> void{
            auto nodeName = node->name;
            auto nodeTransform = node->transformation;
//            auto globalInverse = anim->GetGlobalInverse();

            auto bone = anim->findBone(nodeName);
            if (bone){
                bone->Update(mAnimTime);
                nodeTransform = bone->GetLocalTransform();
            }

            auto globalTransform = parentTransform * nodeTransform;
            auto boneInfoMap = ac.boneMap;
            if (boneInfoMap.find(nodeName) != boneInfoMap.end()){
                auto index = boneInfoMap[nodeName].id;
                auto invBind = boneInfoMap[nodeName].offset;
                // boneMats[index] = globalInverse * globalTransform * invBind;
                boneMats[index] = globalTransform * invBind;
            }

            for(auto i = 0u; i < node->childrenCount; i++){
                calculateBoneTransform(&node->children[i], globalTransform);
            }
        };

        calculateBoneTransform(&ac.boneNode, glm::mat4 (1.f));
        if (boneMatBuf){
            boneMatBuf->update(boneMats);
        };
    }

} // rs