//
// Created by lenovo on 5/23/2024.
//

#include "Engine.h"

namespace yic {

    Engine::Engine() = default;

    bool Engine::run() {
        vkWindow::get();
        mRhi = std::make_unique<vkRhi>();

        [&, rhi_thread = [&]() {
            return std::make_unique<std::thread>([&] {
                try {
                    mRhi->run();
                } catch (const vk::SystemError &e) {
                    std::cerr << e.what() << "\n";
                }
            });
        }(), window_main_thread = [&]() {
            return !vkWindow::run();
        }()
        ]() {
            if (!window_main_thread)
                mRhi->setRunCondition();

            if (rhi_thread->joinable())
                rhi_thread->join();
        }();

        return true;
    }

} // yic