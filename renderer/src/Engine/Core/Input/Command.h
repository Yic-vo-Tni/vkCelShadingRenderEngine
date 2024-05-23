//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_COMMAND_H
#define VKCELSHADINGRENDERER_COMMAND_H

namespace yic {

    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute()  = 0;
        virtual void undo() = 0;
        [[nodiscard]] virtual std::string getName() const = 0;
    };

    class ExitCommand : public Command{
    public:
        explicit ExitCommand(GLFWwindow* window) : mWindow(window), previousState(false) {};

        void execute() override{
            previousState = glfwWindowShouldClose(mWindow);
            glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
        }

        void undo() override{
            glfwSetWindowShouldClose(mWindow, previousState);
        }

        [[nodiscard]] std::string getName() const override{
            return "exit";
        }


    private:
        GLFWwindow *mWindow;
        int previousState;
    };

    class TestCommand : public Command{
    public:
        explicit TestCommand(GLFWwindow* w) : mWindow(w), previousState(false){};

        void execute() override{
           // if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
               // int width, height;
                glfwGetWindowSize(mWindow, &width, &height);
                glfwSetWindowSize(mWindow, width + 10, height + 10);
            //}
        }

        void undo() override{
            glfwSetWindowSize(mWindow, width - 10, height - 10);
        }

        [[nodiscard]] std::string getName() const override{
            return "test";
        }

    private:
        GLFWwindow *mWindow;
        int previousState;
        int width, height;
    };

} // yic

#endif //VKCELSHADINGRENDERER_COMMAND_H
