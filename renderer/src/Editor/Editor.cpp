//
// Created by lenovo on 5/23/2024.
//

#include "Editor.h"

namespace yic {

    Editor::Editor() {
        mEngine = std::make_unique<Engine>();
    }

    void Editor::run() {
        mEngine->run();
    }

} // yic