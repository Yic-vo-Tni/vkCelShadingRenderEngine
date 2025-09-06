//
// Created by lenovo on 10/31/2024.
//

#include "Core/DispatchSystem/SystemHub.h"

#include "ResourceSystem.h"
#include "Saba/Base/Time.h"
#include <saba/src/Saba/Model/MMD/VMDAnimation.h>

#include <oneapi/tbb/task_scheduler_observer.h>

namespace rs {
    class ScopedHighPriorityTBB {
    public:
        ScopedHighPriorityTBB() {
            ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        }

        ~ScopedHighPriorityTBB() {
            ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
        }
    };


    ResourceSystem::ResourceSystem(entt::registry& registry) : ecs(registry) {
        mLoader = std::make_unique<Loader>(ecs);
        mAnimator = std::make_unique<Animator>();
    }

    ResourceSystem::~ResourceSystem() {
        mLoader.reset();
    }

    auto ResourceSystem::frame() -> void {
        const double time = saba::GetTime();
        double elapsed = time - mSaveTime;

        if (elapsed > 0.5f){
            elapsed = 1.f / 30.f;
        }
        mSaveTime = time;
        mElapsed = static_cast<float>(elapsed);

        const auto view = ecs.view<const vot::BasicInfoComponent, vot::VertexDataComponent, vot::AnimationComponent, vot::RenderComponent>();
        const auto count = std::distance(view.begin(), view.end());

        {
            ScopedHighPriorityTBB _prio;
            oneapi::tbb::task_arena arena(6);
            arena.execute([&] {
                oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, count),
                                          [&](const oneapi::tbb::blocked_range<size_t> &r) {
                                              for (size_t i = r.begin(); i != r.end(); ++i) {
                                                  auto it = std::next(view.begin(), i);
                                                  const auto entity = *it;
                                                  auto &info = view.get<const vot::BasicInfoComponent>(entity);
                                                  auto &vc = view.get<vot::VertexDataComponent>(entity);
                                                  auto &ac = view.get<vot::AnimationComponent>(entity);
                                                  auto &rc = view.get<vot::RenderComponent>(entity);

                                                  if (!ac.enableAnim) continue;

                                                  auto playLogic = [&] {
                                                     // if (!vc.isMMD) {
                                                      if (vc.type == vot::eAssimp) {
                                                          mAnimator->sampleAnimation(1.f / GLOBAL::fps, ac);
                                                      } else {
                                                          const auto t = ac.animTime += static_cast<float>(elapsed);
                                                          vc.pmx->BeginAnimation();
                                                          vc.pmx->UpdateAllAnimation(
                                                              ac.vmd.second.get(), t * 30.f, mElapsed);
                                                          vc.pmx->EndAnimation();

                                                          mAnimator->sampleVmd(vc, rc);
                                                      }
                                                  };

                                                  if (GLOBAL::playAllAnim || info.playAnimation) {
                                                      playLogic();
                                                  }
                                              }
                                          });
            });
        }

        if (GLOBAL::playAllAnim) {
            if (!mLoader->mAudio->isPlaying()) {
                if (mLoader->mAudio->isPausedState()) {
                    mLoader->mAudio->resume();
                } else {
                    mLoader->mAudio->play();
                }
            }
        } else {
            if (mLoader->mAudio->isPlaying()) {
                mLoader->mAudio->pause();
            }
        }
    }

    auto ResourceSystem::frameUpdate() -> void {
        ecs.view<const vot::BasicInfoComponent, vot::VertexDataComponent, vot::AnimationComponent, vot::RenderComponent>()
                .each([&](entt::entity, const vot::BasicInfoComponent &info, vot::VertexDataComponent &vc,
                          vot::AnimationComponent &ac, vot::RenderComponent &rc) {
                    if (ac.enableAnim && (GLOBAL::playAllAnim || info.playAnimation)) {
                        //if (!vc.isMMD) {
                        if (vc.type == vot::eAssimp) {
                            mAnimator->sampleAnimation(1.f / GLOBAL::fps, ac);
                        } else {
                            //mAnimator->syncVmd(vc, rc);
                        }
                    }
                });
    }


} // rs