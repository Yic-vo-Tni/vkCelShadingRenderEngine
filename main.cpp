
#include "renderer/src/Application.h"

int main() {
    yic::Application app{};

    try {
        app.run();
    } catch (std::exception& e){
        std::cerr << e.what() << "\n";
    }

    return 0;
}
