//
// Created by lenovo on 5/23/2024.
//

#include "Engine.h"

namespace yic {

    bool Engine::run() {
        [&,
                window_run = [&]() {
                    return !vkWindow::run();
                }()
        ]() {
            try {

            }catch (const std::exception& e){
                std::cerr << "Exception caught in run loop: " << e.what() << "\n";
                return false;
            }
            return true;
        }();

        return true;
    }

} // yic