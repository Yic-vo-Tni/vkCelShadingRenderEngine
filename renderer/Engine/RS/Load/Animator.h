//
// Created by lenovo on 6/12/2025.
//

#ifndef VKCELSHADINGRENDERER_ANIMATOR_H
#define VKCELSHADINGRENDERER_ANIMATOR_H

#include "Animation.h"

namespace rs {

    class Animator {
    public:
        Animator() = default;
        ~Animator() = default;

        auto sampleAnimation(float deltaTime, vot::AnimationComponent& ac) -> void;
    private:
        float mAnimTime{};
    };

} // rs

#endif //VKCELSHADINGRENDERER_ANIMATOR_H
