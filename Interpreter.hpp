#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "Token.hpp"
#include <vector>
#include <string>
#include <map>


class Interpreter {
    std::map<std::string, std::pair<std::string, std::string>> variables;

public:
    void execute(const std::string& line);

private:
    void handleVariableDeclaration(const std::vector<Token>& tokens);
    void handleRunCommand(const std::string& filePath);
    std::string handleSendCommand(const std::vector<Token>& tokens);
    std::vector<Token> tokenize(const std::string& line);
};

#endif