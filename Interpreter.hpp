#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include "Token.hpp"

class Interpreter {
public:
    void execute(const std::string& line);  // Declare execute function
    std::vector<Token> tokenize(const std::string& line);  // Declare tokenize function
    std::string handleSendCommand(const std::vector<Token>& tokens);  // Declare handleSendCommand
    void handleRunCommand(const std::string& filePath);  // Declare handleRunCommand
    void handleVariableDeclaration(const std::vector<Token>& tokens);  // Declare handleVariableDeclaration

private:
    std::unordered_map<std::string, std::pair<std::string, std::string>> variables;
};

#endif