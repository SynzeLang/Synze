#include "Interpreter.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <cmath>

void Interpreter::execute(const std::string& line) {
    std::vector<Token> tokens = tokenize(line);
    static int currentIndentationLevel = 0;
    static std::vector<std::string> bufferedFunctionLines;
    static bool capturingFunction = false;
    static std::string currentFunctionName;
    static std::vector<std::string> currentFunctionParams;

    int lineIndentation = getIndentationLevel(line);

    if (capturingFunction && lineIndentation <= currentIndentationLevel) {
        functions[currentFunctionName] = { currentFunctionParams, bufferedFunctionLines };
        bufferedFunctionLines.clear();
        capturingFunction = false;
    }

    if (tokens.empty()) return;
    if (tokens[0].type == COMMENT) return;

    if (tokens[0].type == FUNC) {
        if (tokens.size() < 2 || tokens[1].type != IDENTIFIER) {
            throw std::runtime_error("Invalid function definition. Syntax: func name param1, param2");
        }

        currentFunctionName = tokens[1].value;

        currentFunctionParams.clear();
        for (size_t i = 2; i < tokens.size(); ++i) {
            if (tokens[i].type == IDENTIFIER) {
                currentFunctionParams.push_back(tokens[i].value);
            } else if (tokens[i].value != ",") {
                throw std::runtime_error("Invalid parameter syntax in function definition.");
            }
        }

        capturingFunction = true;
        currentIndentationLevel = lineIndentation;
        return;
    }

    if (capturingFunction) {
        if (lineIndentation > currentIndentationLevel) {
            bufferedFunctionLines.push_back(trim(line));
        } else {
            throw std::runtime_error("Unexpected indentation in function body.");
        }
        return;
    }

    if (functions.find(tokens[0].value) != functions.end()) {
        handleFunctionCall(tokens);
    }
    else if (tokens[0].type == SEND && tokens.size() > 1) {
        std::string output = handleSendCommand(tokens);
        std::cout << output << std::endl;
    }
    else if (tokens[0].type == RUN && tokens.size() == 2) {
        handleRunCommand(tokens[1].value);
    }
    else if (tokens[0].type == IDENTIFIER && tokens.size() >= 3 && tokens[1].type == ASSIGNMENT) {
        handleVariableDeclaration(tokens);
    }
    else if (tokens[0].type == EXIT) {
        std::cout << "\x1B[2JExiting the interpreter." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(750));
        std::cout << "\x1B[2JGoodbye!" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(750));
        exit(0);
    }
}

void Interpreter::handleVariableDeclaration(const std::vector<Token>& tokens) {
    std::string varName = tokens[0].value;
    Token valueToken = tokens[2];

    if (variables.find(varName) != variables.end()) {
        std::cerr << "Warning: Variable '" << varName
                  << "' already declared. Overwriting the previous value." << std::endl;
    }

    if (valueToken.type == STRING_LITERAL) {
        variables[varName] = { "string", valueToken.value };
    } else if (valueToken.type == NUMBER) {
        variables[varName] = { "number", valueToken.value };
    } else if (valueToken.value == "true" || valueToken.value == "false") {
        variables[varName] = { "boolean", valueToken.value };
    } else if (valueToken.value == "input") {
        std::string inputValue;
        std::getline(std::cin, inputValue);

        if (inputValue == "true" || inputValue == "false") {
            variables[varName] = { "boolean", inputValue };
        } else if (std::all_of(inputValue.begin(), inputValue.end(),
                               [](char c) { return std::isdigit(c) || c == '.'; })) {
            variables[varName] = { "number", inputValue };
        } else {
            variables[varName] = { "string", inputValue };
        }
    } else if (valueToken.type == IDENTIFIER) {
        auto it = variables.find(valueToken.value);
        if (it == variables.end()) {
            throw std::runtime_error("Undefined variable: " + valueToken.value);
        }
        variables[varName] = it->second;
    } else {
        throw std::runtime_error("Unsupported value type for variable declaration.");
    }
}

void Interpreter::handleRunCommand(const std::string& filePath) {
    auto start = std::chrono::high_resolution_clock::now();

    std::string normalizedPath = filePath;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

    if (normalizedPath.size() < 7 || normalizedPath.substr(normalizedPath.size() - 6) != ".synze") {
        throw std::runtime_error("Invalid file extension. Expected .synze");
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
    std::cout << "\nSuccessfully executed file: " << normalizedPath << " in " << duration << "ms\n" << std::endl;
}

std::string Interpreter::handleSendCommand(const std::vector<Token>& tokens) {
    std::ostringstream output;
    double mathResult = 0;
    bool isMathMode = false;
    char currentOperator = '+';
    bool mathStarted = false;
    bool insideString = false;
    std::ostringstream tempString;

    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& tokenValue = tokens[i].value;

        if (tokens[i].type == STRING_LITERAL) {
            if (!insideString) {
                insideString = true;
                tempString.str("");
            }

            for (size_t j = 0; j < tokens[i].value.size(); ++j) {
                if (tokens[i].value[j] == '{') {
                    size_t k = j + 1;
                    std::string varName;
                    while (k < tokens[i].value.size() && tokens[i].value[k] != '}') {
                        varName += tokens[i].value[k++];
                    }
                    if (k == tokens[i].value.size() || tokens[i].value[k] != '}') {
                        throw std::runtime_error("Unmatched '{' in string.");
                    }
                    j = k;

                    auto it = variables.find(varName);
                    if (it == variables.end()) {
                        throw std::runtime_error("Undefined variable: " + varName);
                    }
                    tempString << it->second.second;
                } else {
                    tempString << tokens[i].value[j];
                }
            }
        } 
        else if (tokens[i].type == IDENTIFIER) {
            auto it = variables.find(tokenValue);
            if (it == variables.end()) {
                throw std::runtime_error("Undefined variable: " + tokenValue);
            }

            const std::string& varType = it->second.first;
            const std::string& varValue = it->second.second;

            if (insideString) {
                tempString << varValue;
            } else if (varType == "number") {
                double value = std::stod(varValue);
                if (!mathStarted) {
                    mathResult = value;
                    mathStarted = true;
                } else if (isMathMode) {
                    switch (currentOperator) {
                        case '+': mathResult += value; break;
                        case '-': mathResult -= value; break;
                        case '*': mathResult *= value; break;
                        case '^': mathResult = std::pow(mathResult, value); break;
                        case '/':
                            if (value == 0) throw std::runtime_error("Division by zero.");
                            mathResult /= value;
                            break;
                    }
                }
            } else {
                output << varValue;
            }
        }
        else if (tokens[i].type == NUMBER) {
            double value = std::stod(tokenValue);
            if (!mathStarted) {
                mathResult = value;
                mathStarted = true;
            } else if (isMathMode) {
                switch (currentOperator) {
                    case '+': mathResult += value; break;
                    case '-': mathResult -= value; break;
                    case '*': mathResult *= value; break;
                    case '^': mathResult = std::pow(mathResult, value); break;
                    case '/':
                        if (value == 0) throw std::runtime_error("Division by zero.");
                        mathResult /= value;
                        break;
                }
            }
        } 
        else if (tokens[i].type == OPERATOR && tokenValue == "+") {
            if (insideString) {
                tempString << "";
            } else {
                currentOperator = tokenValue[0];
                isMathMode = true;
            }
        }
        else if (tokens[i].type == OPERATOR) {
            currentOperator = tokenValue[0];
            isMathMode = true;
        }
        else if (tokenValue == "input") {
            if (i + 1 >= tokens.size() || tokens[i + 1].type != IDENTIFIER) {
                throw std::runtime_error("Expected a variable name after 'input'.");
            }

            const std::string& varName = tokens[i + 1].value;
            std::string userInput;

            std::getline(std::cin, userInput);

            variables[varName] = {"string", userInput};

            ++i;
        }


        else if (tokenValue[0] == '{') {
            throw std::runtime_error("Invalid use of curly braces '{'.");
        }

        if (tokenValue == "#") {
            break;
        }
    }

    if (insideString) {
        output << tempString.str();
    }

    if (mathStarted) {
        output << mathResult;
    }

    return output.str();
}

double Interpreter::evaluateExpression(const std::string& expr) {
    std::istringstream exprStream(expr);
    double result = 0;
    double currentValue;
    char op = '+';

    while (exprStream >> currentValue) {
        if (exprStream >> op) {
            switch (op) {
                case '+': result += currentValue; break;
                case '-': result -= currentValue; break;
                case '*': result *= currentValue; break;
                case '^': result = std::pow(result, currentValue); break;
                case '/':
                    if (currentValue == 0) throw std::runtime_error("Division by zero.");
                    result /= currentValue;
                    break;
                default:
                    throw std::runtime_error("Invalid operator in expression.");
            }
        } else {
            result = currentValue;
        }
    }
    return result;
}

void Interpreter::handleFunctionCall(const std::vector<Token>& tokens) {
    const std::string& funcName = tokens[0].value;

    if (functions.find(funcName) == functions.end()) {
        throw std::runtime_error("Undefined function: " + funcName);
    }

    const auto& func = functions[funcName];
    const auto& paramNames = func.first;
    const auto& funcBody = func.second;

    std::vector<std::string> args;
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i].type == IDENTIFIER || tokens[i].type == NUMBER || tokens[i].type == STRING_LITERAL) {
            args.push_back(tokens[i].value);
        } else if (tokens[i].type == OPERATOR && tokens[i].value == ",") {
            continue;
        } else {
            throw std::runtime_error("Invalid syntax in function call.");
        }
    }

    if (args.size() != paramNames.size()) {
        throw std::runtime_error("Function '" + funcName + "' expects " +
                                 std::to_string(paramNames.size()) + " arguments, but " +
                                 std::to_string(args.size()) + " were provided.");
    }

    auto globalVars = variables;
    std::unordered_map<std::string, std::pair<std::string, std::string>> localVars = variables;

    for (size_t i = 0; i < paramNames.size(); ++i) {
        const std::string& argValue = args[i];

        if (variables.find(argValue) != variables.end()) {
            localVars[paramNames[i]] = variables[argValue];
        } else if (std::all_of(argValue.begin(), argValue.end(), ::isdigit) ||
                   (argValue[0] == '-' && argValue.size() > 1 && std::isdigit(argValue[1]))) {
            localVars[paramNames[i]] = { "number", argValue };
        } else {
            localVars[paramNames[i]] = { "string", argValue };
        }
    }

    variables = localVars;
    for (const auto& line : funcBody) {
        execute(line);
    }

    variables = globalVars;
}

int Interpreter::getIndentationLevel(const std::string& line) {
    int level = 0;
    for (char c : line) {
        if (c == ' ') level++;
        else if (c == '\t') level += 4;
        else break;
    }
    return level;
}

std::string Interpreter::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

std::vector<Token> Interpreter::tokenize(const std::string& line) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < line.length()) {
        if (std::isspace(line[i])) {
            i++;
            continue;
        }

        if (line[i] == '#') {
            if (!tokens.empty() && tokens.back().type == SEND) {
                tokens.push_back({ STRING_LITERAL, "#" });
                i++;
                continue;
            }
            tokens.push_back({ COMMENT, line.substr(i) });
            break;
        }
        else if (line[i] == ',') {
            tokens.push_back({ OPERATOR, "," });
            i++;
            continue;
        }
        else if (line.substr(i, 4) == "send") {
            size_t nextCharPos = i + 4;
            if (nextCharPos >= line.length() || !std::isspace(line[nextCharPos])) {
                throw std::runtime_error("Invalid syntax: 'send' must be followed by a space.");
            }
            tokens.push_back({ SEND, "send" });
            i += 4;
        }
        else if (line.substr(i, 4) == "func") {
            tokens.push_back({ FUNC, "func" });
            i += 4;
            continue;
        }
        else if (line.substr(i, 3) == "run") {
            tokens.push_back({ RUN, "run" });
            i += 3;
            while (i < line.length() && std::isspace(line[i])) i++;
            size_t pathStart = i;
            while (i < line.length() && !std::isspace(line[i])) i++;
            tokens.push_back({ STRING_LITERAL, line.substr(pathStart, i - pathStart) });
        }
        else if (line.substr(i, 8) == "variable") {
            tokens.push_back({ VARIABLE, "variable" });
            i += 8;
        }
        else if (line.substr(i, 4) == "exit") {
            tokens.push_back({ EXIT, "exit" });
            i += 4;
        }
        else if (line[i] == '"') {
            std::string literal;
            i++;
            while (i < line.length() && line[i] != '"') {
                if (line[i] == '\\' && i + 1 < line.length()) {
                    switch (line[i + 1]) {
                        case 'n': literal += '\n'; break;
                        case 't': literal += '\t'; break;
                        case '\\': literal += '\\'; break;
                        case '"': literal += '"'; break;
                        default: literal += line[i + 1]; break;
                    }
                    i++;
                } else {
                    literal += line[i];
                }
                i++;
            }
            if (i >= line.length() || line[i] != '"') {
                throw std::runtime_error("Unterminated string literal");
            }
            tokens.push_back({ STRING_LITERAL, literal });
            i++;
        }
        else if (line[i] == '-' && (i == 0 || tokens.back().type == OPERATOR || tokens.back().type == ASSIGNMENT || tokens.back().type == SEND)) {
            size_t start = i++;
            while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.')) i++;
            if (start + 1 == i) {
                tokens.push_back({ OPERATOR, "-" });
            } else {
                tokens.push_back({ NUMBER, line.substr(start, i - start) });
            }
        }
        else if (std::isdigit(line[i])) {
            size_t start = i;
            while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.')) i++;
            tokens.push_back({ NUMBER, line.substr(start, i - start) });
        }
        else if (std::isalpha(line[i]) || line[i] == '_') {
            size_t start = i;
            while (i < line.length() && (std::isalnum(line[i]) || line[i] == '_')) i++;
            tokens.push_back({ IDENTIFIER, line.substr(start, i - start) });
        }
        else if (line[i] == '=' || line[i] == '+' || line[i] == '-' || line[i] == '*' || line[i] == '^' || line[i] == '/') {
            tokens.push_back({ line[i] == '=' ? ASSIGNMENT : OPERATOR, std::string(1, line[i]) });
            i++;
        } else {
            throw std::runtime_error("Invalid token at: " + std::string(1, line[i]));
        }
    }

    return tokens;
}