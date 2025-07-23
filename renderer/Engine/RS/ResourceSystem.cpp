//
// Created by lenovo on 10/31/2024.
//

#include "Core/DispatchSystem/SystemHub.h"

#include "ResourceSystem.h"
#include "Saba/Base/Time.h"
#include <saba/src/Saba/Model/MMD/VMDAnimation.h>

namespace rs {

    ResourceSystem::ResourceSystem(entt::registry& registry) : ecs(registry) {
        mLoader = std::make_unique<Loader>(ecs);
        mAnimator = std::make_unique<Animator>();
    }

    ResourceSystem::~ResourceSystem() {
        mLoader.reset();
    }

    auto ResourceSystem::frame() -> void {
        double time = saba::GetTime();
        double elapsed = time - mSaveTime;
//        if (elapsed > 1.f / 30.f){
//            elapsed = 1.f / 30.f;
//        }
        if (elapsed > 0.5f){
            elapsed = 1.f / 30.f;
        }
        mSaveTime = time;
        mElapsed = float(elapsed);

        ecs.view<const vot::BasicInfoComponent, vot::VertexDataComponent, vot::AnimationComponent, vot::RenderComponent>()
                .each([&](entt::entity, const vot::BasicInfoComponent &info, vot::VertexDataComponent &vc,
                          vot::AnimationComponent &ac, vot::RenderComponent &rc) {
                    if (info.playAnimation) {
                        if (!vc.isMMD) {
                            mAnimator->sampleAnimation(1.f / GLOBAL::fps, ac);
                        } else {
                            mAnimTime += float(elapsed);
                            vc.pmx->BeginAnimation();
                            vc.pmx->UpdateAllAnimation(ac.vmd.second.get(), mAnimTime * 30.f, mElapsed);
                            vc.pmx->EndAnimation();

                            mLoader->mMmdLoader->updateAnim(vc, rc);

                            mLoader->mAudio->play();
                        }
                    }
                });
    }



} // rs