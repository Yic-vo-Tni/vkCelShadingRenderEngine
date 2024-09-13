//
// Created by lenovo on 5/23/2024.
//

#include "InputHandlers.h"

#include <utility>

namespace yic {

    InputHandlers::InputHandlers() : mWindow{mg::SystemHub.val<ev::hVkRenderContext>(toolkit::enum_name(RenderPhase::ePrimary)).window}{
        registerCmd(GLFW_KEY_ESCAPE, "exit", [&]() { return std::make_shared<ExitCommand>(mWindow); });
        registerCmd(GLFW_KEY_SPACE, "test", [&]() { return std::make_shared<TestCommand>(mWindow); });

        mg::SystemHub.subscribe([&](const et::glKeyInput& input){
            auto inputHandle = InputHandlers::get();
            inputHandle->handleUserInput(input.key.value(), input.action.value());
        });

        callback(mWindow);
    }

    void InputHandlers::withDraw() {
        auto inst = get();

        auto ctrl = glfwGetKey(inst->mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
        auto z = glfwGetKey(inst->mWindow, GLFW_KEY_Z) == GLFW_PRESS;

        if (ctrl && z && !inst->undo){
            inst->undoLastCommand();
            inst->undo = true;
        } else if (!ctrl || !z){
            inst->undo = false;
        }

        inst->globalCamera();
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

    auto InputHandlers::globalCamera() -> void {
        if (!EventBus::val<et::frameTime>().frameTime_exists())
            return;
        auto cameraSpeed = EventBus::val<et::frameTime>().frameTime_v() * sc::globalCamera.getDynamicSpeed() * 50;

        auto glfwKeyPress = [&](int key){
            return (glfwGetKey(mWindow, key) == GLFW_PRESS);
        };
        auto glfwMouseButtonPress = [&](int button){
            return (glfwGetMouseButton(mWindow, button) == GLFW_PRESS);
        };
        auto glfwMouseButtonRelease = [&](int button){
            return (glfwGetMouseButton(mWindow, button) == GLFW_RELEASE);
        };

        if (glfwKeyPress(GLFW_KEY_W)){
            sc::globalCamera.getPosition() += cameraSpeed * sc::globalCamera.getCameraFront();
        }
        if (glfwKeyPress(GLFW_KEY_S))
            sc::globalCamera.getPosition() -= cameraSpeed * sc::globalCamera.getCameraFront();
        if (glfwKeyPress(GLFW_KEY_A))
            sc::globalCamera.getPosition() -= glm::normalize(glm::cross(sc::globalCamera.getCameraFront(), sc::globalCamera.getCameraUp())) * cameraSpeed;
        if (glfwKeyPress(GLFW_KEY_D))
            sc::globalCamera.getPosition() += glm::normalize(glm::cross(sc::globalCamera.getCameraFront(), sc::globalCamera.getCameraUp())) * cameraSpeed;


        if (glfwMouseButtonPress(GLFW_MOUSE_BUTTON_RIGHT)){
            if (firstClick){
                glfwSetCursorPos(mWindow, xLast, yLast);
                firstClick = false;
            }
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xPos, double yPos){
                sc::globalCamera.mouseCallback(xPos, yPos);
            });
            glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xOffset, double yOffset){
                sc::globalCamera.scrollCallback(xOffset, yOffset);
            });

        }
        if (glfwMouseButtonRelease(GLFW_MOUSE_BUTTON_RIGHT)){
            glfwGetCursorPos(mWindow, &xLast, &yLast);
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cameraCallback(mWindow);
            firstClick = true;
            sc::globalCamera.firstMouse = true;
        }
    }


} // yic