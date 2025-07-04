#include <iostream>

#include "Engine.h"

int main() {
    Engine engine{};

    try {
        engine.run();
    } catch (std::exception& exception) {
        std::cerr << exception.what() << '\n';
    }

    return 0;
}
