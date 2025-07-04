//
// Created by lenovo on 5/23/2025.
//

#ifndef VKCELSHADINGRENDERER_ANIMATION_H
#define VKCELSHADINGRENDERER_ANIMATION_H

#include "assimp/scene.h"

#include "assimp/matrix4x4.h"
#include "assimp/quaternion.h"

#include "Node.h"


namespace rs {

    class Animation {
    public:
        Animation(const aiAnimation* aiAnim, vot::AnimationComponent& ac);
        Animation(const std::shared_ptr<vmd::VmdMotion>& vmd, vot::AnimationComponent& ac);
        ~Animation();

        auto readMissingBones(const aiAnimation* aiAnim, vot::AnimationComponent& ac) -> void;
        [[nodiscard]] auto getDuration() const { return mDuration;}
        [[nodiscard]] auto getTicksPerSecond() const { return mTicksPerSecond; }
        [[nodiscard]] auto findBone(const vot::string& name) -> Bone*;
    private:
    private:
        float mDuration;
        float mTicksPerSecond;
        vot::vector<Bone> mBones;
        glm::mat4 mGlobalInverse{};
    };





} // rs

#endif //VKCELSHADINGRENDERER_ANIMATION_H
