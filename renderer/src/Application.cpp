//
// Created by lenovo on 5/21/2024.
//

#include "Application.h"

namespace yic {

    Application::Application() {
        mEditor = std::make_unique<Editor>();
    }

    void Application::run() {
        mEditor->run();
    }


} // yic