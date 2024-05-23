//
// Created by lenovo on 5/23/2024.
//

#include "Engine.h"

namespace yic {

    Engine::Engine() = default;

    bool Engine::run() {
        mRhi = std::make_unique<vkRhi>();

        [&, rhi_run = [&]() {
            return mRhi->run();
        }(), window_run = [&]() {
            return !vkWindow::run();
        }()
        ]() {
                if (!window_run)
                    mRhi->setRunCondition();
        }();

        return true;
    }

} // yic