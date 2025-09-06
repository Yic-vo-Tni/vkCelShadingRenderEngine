//
// Created by lenovo on 6/12/2025.
//

#ifndef VKCELSHADINGRENDERER_ANIMATOR_H
#define VKCELSHADINGRENDERER_ANIMATOR_H

#include "Animation.h"
#include "Saba/Model/MMD/VMDFile.h"

namespace rs {

    class Animator {
    public:
        Animator() = default;
        ~Animator() = default;

        //assimp
        auto sampleAnimation(float deltaTime, vot::AnimationComponent& ac) -> void;
        // vmd
        auto bindVmd(const std::pair<vot::string, saba::VMDFile>& vmdFile, const vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void;
        auto sampleVmd(vot::VertexDataComponent& vc, const vot::RenderComponent& rc) -> void;

    private:
        // assimp
        float mAnimTime{};
        // vmd
    };

} // rs

#endif //VKCELSHADINGRENDERER_ANIMATOR_H
