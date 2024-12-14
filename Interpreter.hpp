#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "Token.hpp"

class Interpreter {
public:
    void execute(const std::string& line); // Main execution loop
    std::vector<Token> tokenize(const std::string& line); // Tokenizes a line of code
    std::string handleSendCommand(const std::vector<Token>& tokens); // Handles send command
    void handleRunCommand(const std::string& filePath); // Handles run command
    void handleVariableDeclaration(const std::vector<Token>& tokens); // Handles variables

private:
    // Functions data structure: maps function name to (parameters, body)
    std::unordered_map<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>> functions;
    // Variables data structure: maps variable name to (type, value)
    std::unordered_map<std::string, std::pair<std::string, std::string>> variables;

    // Helpers
    void handleFunctionDefinition(const std::vector<Token>& tokens); // Handles func keyword
    void handleFunctionCall(const std::vector<Token>& tokens); // Handles function calls
    int getIndentationLevel(const std::string& line); // Detects indentation level
    std::string trim(const std::string& str); // Trims whitespace

    double evaluateExpression(const std::string& expr); // Evaluates mathematical expressions
};

#endif