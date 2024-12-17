#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "Token.hpp"

class Interpreter {
public:
    void execute(const std::string& line);
    std::vector<Token> tokenize(const std::string& line);
    std::string handleSendCommand(const std::vector<Token>& tokens);
    void handleRunCommand(const std::string& filePath);
    void handleVariableDeclaration(const std::vector<Token>& tokens);

private:
    std::unordered_map<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>> functions;
    std::unordered_map<std::string, std::pair<std::string, std::string>> variables;

    void handleFunctionCall(const std::vector<Token>& tokens);
    int getIndentationLevel(const std::string& line);
    std::string trim(const std::string& str);

    double evaluateExpression(const std::string& expr);
};

#endif