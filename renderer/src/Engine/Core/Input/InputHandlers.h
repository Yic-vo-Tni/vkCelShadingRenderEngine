//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_INPUTHANDLERS_H
#define VKCELSHADINGRENDERER_INPUTHANDLERS_H

#include "Engine/Core/Callback/GlfwCallback.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Core/Input/CommandParser.h"
#include "Engine/ECS/Camera/Camera.h"

namespace yic {

    class InputHandlers {
    public:
        explicit InputHandlers();
        vkGet auto get = [](){ return Singleton<InputHandlers>::get();};

        static void withDraw();
    private:
        auto globalCamera() -> void;
        void handleUserInput(int key, int action);
        void registerCmd(int key, const std::string &name, std::function<std::shared_ptr<Command>()> commandFactory);
        void undoLastCommand();


    private:
        GLFWwindow* mWindow;
        bool undo = false;
        std::unordered_map<int, std::function<std::shared_ptr<Command>()>> mCommands;
        std::stack<std::shared_ptr<Command>> mCommandHistory;

        bool firstClick = true;
        double xLast{}, yLast{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_INPUTHANDLERS_H
