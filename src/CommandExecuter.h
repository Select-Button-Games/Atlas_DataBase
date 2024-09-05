#pragma once
#include "Query_Parser.h"
#include <fstream>
#include <string>

class CommandExecutor {
public:
    explicit CommandExecutor(QueryParser& parser) : parser(parser) {}

    bool executeCommandsFromFile(const std::string& fileName);

private:
    QueryParser& parser;
};

bool CommandExecutor::executeCommandsFromFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return false;
    }

    std::string command;
    while (std::getline(file, command)) {
        if (!parser.executeCommand(command)) {
            std::cerr << "Failed to execute command: " << command << std::endl;
            return false;
        }
    }

    file.close();
    return true;
}
