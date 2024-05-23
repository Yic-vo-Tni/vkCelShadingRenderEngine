//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_INPUTHANDLERS_H
#define VKCELSHADINGRENDERER_INPUTHANDLERS_H

#include "Engine/Core/Event/Event.h"
#include "Engine/Core/Input/CommandParser.h"

namespace yic {

    class InputHandlers {
    public:
        explicit InputHandlers(GLFWwindow *w);
        vkGet auto get = [](GLFWwindow* w){ return Singleton<InputHandlers>::get(w);};

        void withDraw();
    private:
        void handleUserInput(int key, int action);
        void registerCmd(int key, const std::string &name, std::function<std::shared_ptr<Command>()> commandFactory);
        void undoLastCommand();

        GLFWwindow* mWindow;
        bool undo = false;
        std::unordered_map<int, std::function<std::shared_ptr<Command>()>> mCommands;
        std::stack<std::shared_ptr<Command>> mCommandHistory;


   //     CommandParser mCommandParser;


//        std::queue<std::string> mInputQueue;
//        std::mutex mQueueMutex;
//        std::condition_variable mQueueCondVar;
//        bool mRunning{true};
    };

} // yic

#endif //VKCELSHADINGRENDERER_INPUTHANDLERS_H
