//
// Created by lenovo on 5/23/2024.
//

#include "CommandParser.h"

#include <utility>

namespace yic {

    void CommandParser::registerCommand(const std::string &name, std::shared_ptr<Command> command) {
        mCommands[name] = std::move(command);
    }

    std::shared_ptr<Command> CommandParser::parseCommand(const std::string &input) {
        auto it = mCommands.find(input);
        if(it != mCommands.end()){
            return it->second;
        }
        return nullptr;
    }

} // yic