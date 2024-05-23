//
// Created by lenovo on 5/23/2024.
//

#include "InputHandlers.h"

#include <utility>

namespace yic {

    InputHandlers::InputHandlers(GLFWwindow *w) : mWindow{w}{
        registerCmd(GLFW_KEY_ESCAPE, "exit", [&]() { return std::make_shared<ExitCommand>(mWindow); });
        registerCmd(GLFW_KEY_SPACE, "test", [&]() { return std::make_shared<TestCommand>(mWindow); });

//        glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods){
//            auto inputHandlers = InputHandlers::get(window);
//            inputHandlers->handleUserInput(key, action);
//        });

        EventBus::subscribeAuto([&](const EventTypes::KeyInput& input){
           auto inputHandlers = InputHandlers::get(mWindow);
           inputHandlers->handleUserInput(input.key, input.action);
        });
    }

    void InputHandlers::withDraw() {
        auto ctrl = glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
        auto z = glfwGetKey(mWindow, GLFW_KEY_Z) == GLFW_PRESS;

        if (ctrl && z && !undo){
            undoLastCommand();
            undo = true;
        } else if (!ctrl || !z){
            undo = false;
        }
    }

    void InputHandlers::handleUserInput(int key, int action) {
        auto it = mCommands.find(key);
        if (it != mCommands.end() && action == GLFW_PRESS) {
            auto cmd = it->second();
            cmd->execute();
            mCommandHistory.push(cmd);
        }
    }

    void InputHandlers::undoLastCommand() {
        if (!mCommandHistory.empty()) {
            auto lastCommand = mCommandHistory.top();
            lastCommand->undo();
            mCommandHistory.pop();
        }
    }

    void InputHandlers::registerCmd(int key, const std::string &name, std::function<std::shared_ptr<Command>()> commandFactory) {
        mCommands[key] = std::move(commandFactory);
    }



//    void InputHandlers::enqueueUserInput(const std::string &input) {
//        {
//            std::lock_guard<std::mutex> lock(mQueueMutex);
//            mInputQueue.push(input);
//        }
//        mQueueCondVar.notify_one();
//    }
//
//    void InputHandlers::processQueueInput() {
//        std::unique_lock<std::mutex> lock(mQueueMutex);
//        while (!mInputQueue.empty()){
//            std::string input = std::move(mInputQueue.front());
//            mInputQueue.pop();
//            lock.unlock();
//            handleUserInput(input);
//            lock.lock();
//        }
//    }
//
//    void InputHandlers::stop() {
//        {
//            std::lock_guard<std::mutex> lock(mQueueMutex);
//            mRunning = false;
//        }
//        mQueueCondVar.notify_all();
//    }


} // yic