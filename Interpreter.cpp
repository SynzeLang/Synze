#include "Interpreter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <chrono>

void Interpreter::execute(const std::string& line) {
    std::vector<Token> tokens = tokenize(line);
    if (tokens.empty()) return;

    if (tokens[0].type == SEND && tokens.size() > 1) {
        std::string output = handleSendCommand(tokens);
        std::cout << output << std::endl;
    } else if (tokens[0].type == RUN && tokens.size() == 2) {
        handleRunCommand(tokens[1].value);
    } else if (tokens[0].type == VARIABLE && tokens.size() >= 4 && tokens[1].type == IDENTIFIER && tokens[2].type == ASSIGNMENT) {
        handleVariableDeclaration(tokens);
    } else {
        throw std::runtime_error("Invalid command. Use 'help' for more help.");
    }
}

void Interpreter::handleVariableDeclaration(const std::vector<Token>& tokens) {
    std::string varName = tokens[1].value;
    std::string rawValue = tokens[3].value;

    if (tokens[3].type == STRING_LITERAL) {
        variables[varName] = { "string", rawValue };
    } else if (tokens[3].type == NUMBER) {
        variables[varName] = { "number", rawValue };
    } else if (rawValue == "true" || rawValue == "false") {
        variables[varName] = { "boolean", rawValue };
    } else if (tokens[3].type == IDENTIFIER) {
        auto value = variables.find(rawValue);
        if (value != variables.end()) {
            variables[varName] = value->second;
        } else {
            throw std::runtime_error("Undefined variable: " + rawValue);
        }
    } else {
        throw std::runtime_error("Unsupported value type for variable declaration.");
    }
}

void Interpreter::handleRunCommand(const std::string& filePath) {
    auto start = std::chrono::high_resolution_clock::now();

    std::string normalizedPath = filePath;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

    if (normalizedPath.size() < 8 || normalizedPath.substr(normalizedPath.size() - 7) != ".syntax") {
        throw std::runtime_error("Invalid file extension. Expected .syntax");
    }

    std::ifstream file(normalizedPath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + normalizedPath);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || std::all_of(line.begin(), line.end(), ::isspace)) continue;
        try {
            execute(line);
        } catch (const std::exception& e) {
            std::cerr << "Error in line \"" << line << "\": " << e.what() << std::endl;
        }
    }

    file.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Successfully executed file: " << normalizedPath << " in " << duration << "ms" << std::endl;
}

std::string Interpreter::handleSendCommand(const std::vector<Token>& tokens) {
    std::ostringstream output;
    double mathResult = 0;
    bool isMathMode = false;
    char currentOperator = '+';
    bool mathStarted = false;

    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i].type == STRING_LITERAL) {
            output << tokens[i].value;
            isMathMode = false;
        } else if (tokens[i].type == IDENTIFIER || tokens[i].type == NUMBER) {
            double value = 0;
            std::string tokenValue = tokens[i].value;

            if (tokens[i].type == IDENTIFIER) {
                auto it = variables.find(tokenValue);
                if (it == variables.end()) {
                    throw std::runtime_error("Undefined variable: " + tokenValue);
                }
                if (it->second.first == "string") {
                    output << it->second.second;
                    continue;
                }
                value = std::stod(it->second.second);
            } else {
                value = std::stod(tokenValue);
            }

            if (!mathStarted) {
                mathResult = value;
                mathStarted = true;
            } else if (isMathMode) {
                switch (currentOperator) {
                    case '+': mathResult += value; break;
                    case '-': mathResult -= value; break;
                    case '*': mathResult *= value; break;
                    case '/':
                        if (value == 0) throw std::runtime_error("Division by zero.");
                        mathResult /= value;
                        break;
                }
            }
        } else if (tokens[i].type == OPERATOR) {
            currentOperator = tokens[i].value[0];
            isMathMode = true;
            continue;
        }
    }

    std::ostringstream temp;
    temp << std::fixed << std::setprecision(10) << mathResult;
    std::string result = temp.str();
    
    if (result.find('.') != std::string::npos) {
        result = result.substr(0, result.find_last_not_of('0') + 1);
        if (result.back() == '.') {
            result.pop_back();
        }
    }
    
    return result;
}

std::vector<Token> Interpreter::tokenize(const std::string& line) {
    std::vector<Token> tokens;
    size_t i = 0;
    bool expectOperandOrUnaryMinus = true;

    while (i < line.length()) {
        if (std::isspace(line[i])) {
            i++;
            continue;
        }

        if (line.substr(i, 4) == "send") {
            tokens.push_back({ SEND, "send" });
            i += 4;
        } else if (line.substr(i, 3) == "run") {
            tokens.push_back({ RUN, "run" });
            i += 3;
            while (i < line.length() && std::isspace(line[i])) i++;
            size_t pathStart = i;
            while (i < line.length() && !std::isspace(line[i])) i++;
            tokens.push_back({ IDENTIFIER, line.substr(pathStart, i - pathStart) });
        } else if (line.substr(i, 8) == "variable") {
            tokens.push_back({ VARIABLE, "variable" });
            i += 8;
        } else if (line[i] == '"') {
            size_t endQuote = line.find('"', i + 1);
            if (endQuote == std::string::npos) {
                throw std::runtime_error("Unterminated string literal");
            }
            tokens.push_back({ STRING_LITERAL, line.substr(i + 1, endQuote - i - 1) });
            i = endQuote + 1;
            expectOperandOrUnaryMinus = false;
        } else if (std::isdigit(line[i]) || (line[i] == '-' && expectOperandOrUnaryMinus && i + 1 < line.length() && std::isdigit(line[i + 1]))) {
            size_t start = i;
            if (line[i] == '-') i++;
            while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.')) i++;
            tokens.push_back({ NUMBER, line.substr(start, i - start) });
            expectOperandOrUnaryMinus = false;
        } else if (std::isalpha(line[i]) || line[i] == '_') {
            size_t start = i;
            while (i < line.length() && (std::isalnum(line[i]) || line[i] == '_' || line[i] == '/' || line[i] == ':' || line[i] == '.')) i++;
            tokens.push_back({ IDENTIFIER, line.substr(start, i - start) });
            expectOperandOrUnaryMinus = false;
        } else if (line[i] == '=') {
            tokens.push_back({ OPERATOR, "=" });
            i++;
            expectOperandOrUnaryMinus = true;
        } else if (line[i] == '+' || line[i] == '-' || line[i] == '*' || line[i] == '/') {
            if (line[i] == '-' && expectOperandOrUnaryMinus) {
                continue;
            }
            tokens.push_back({ OPERATOR, std::string(1, line[i]) });
            i++;
            expectOperandOrUnaryMinus = true;
        } else {
            throw std::runtime_error("Invalid token");
        }
    }
    return tokens;
}