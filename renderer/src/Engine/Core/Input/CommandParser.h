//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_COMMANDPARSER_H
#define VKCELSHADINGRENDERER_COMMANDPARSER_H

#include "Engine/Core/Input/Command.h"

namespace yic {

    class CommandParser {
    public:
        void registerCommand(const std::string& name, std::shared_ptr<Command> command);
        std::shared_ptr<Command> parseCommand(const std::string& input);

    private:
        std::unordered_map<std::string, std::shared_ptr<Command>> mCommands;
    };

} // yic

#endif //VKCELSHADINGRENDERER_COMMANDPARSER_H
