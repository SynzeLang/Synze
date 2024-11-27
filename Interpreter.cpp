#include "Interpreter.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <chrono>

void Interpreter::execute(const std::string& line) {
    std::vector<Token> tokens = tokenize(line);
    if (tokens.empty()) return;

    std::cout << "\n";
    if (tokens[0].type == SEND && tokens.size() > 1) {
        std::string output = handleSendCommand(tokens);
        std::cout << output << std::endl;
    } else if (tokens[0].type == RUN && tokens.size() == 2) {
        handleRunCommand(tokens[1].value);
    } else if (tokens[0].type == VARIABLE && tokens.size() >= 4 && tokens[1].type == IDENTIFIER && tokens[2].type == ASSIGNMENT) {
        handleVariableDeclaration(tokens);
    } else if (tokens[0].type == EXIT) {
        std::cout << "Exiting the interpreter. Goodbye!" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        exit(0);
    } if (tokens[0].type == HELP) {
        if (tokens.size() == 2) {
            const std::string& command = tokens[1].value;

            if (command == "run") {
                std::cout << "Run Command Help Menu\n"
                          << "Executes a specified file with syntax commands. Syntax:\n"
                          << "  run [file]\n\n"
                          << "- [file]: Path to the .syntax file to be executed.\n"
                          << "  Ensure the file exists and uses the .syntax extension.\n";
            } else if (command == "send") {
                std::cout << "Send Command Help Menu\n"
                          << "Processes and outputs a text message or evaluates expressions. Syntax:\n"
                          << "  send [text or expression]\n\n"
                          << "- [text or expression]: Can be a plain message, mathematical\n"
                          << "  operation, or variable reference.\n";
            } else if (command == "variable") {
                std::cout << "Variable Declaration Help Menu\n"
                          << "Creates or updates a variable. Syntax:\n"
                          << "  variable [name] = [value]\n\n"
                          << "- [name]: Identifier for the variable (letters, numbers, or underscores).\n"
                          << "- [value]: Initial value (string, number, boolean, or another variable).\n";
            } else if (command == "exit") {
                std::cout << "Exit Command Help Menu\n"
                          << "Terminates the interpreter. Syntax:\n"
                          << "  exit\n\n"
                          << "- No additional arguments are required.\n";
            } else {
                std::cout << "Error: No help available for command: " << command << std::endl;
            }
        } else if (tokens.size() == 1) {
            std::cout << "Help Menu\n\n"
                      << "Command Overview:\n"
                      << "  []: Required parameters.\n"
                      << "  (): Optional parameters.\n\n"
                      << "Commands:\n"
                      << "  variable [name] = [value] - Declare or assign a variable.\n"
                      << "  send [text or expression] - Output a message or evaluate an expression.\n"
                      << "  run [file] - Execute a file containing commands.\n"
                      << "  exit - Close the interpreter.\n"
                      << "  help (command) - Display help for a specific command.\n";
        } else {
            std::cout << "Error: Invalid syntax for help command. Use 'help' or 'help [command]'.\n";
        }
    } else {
        throw std::runtime_error("Invalid command. Use 'help' for more help.");
    }
}

void Interpreter::handleVariableDeclaration(const std::vector<Token>& tokens) {
    std::string varName = tokens[1].value;

    Token valueToken = tokens[3];

    if (valueToken.type == OPERATOR && valueToken.value == "-" &&
        tokens.size() > 4 && tokens[4].type == NUMBER) {
        valueToken.type = NUMBER;
        valueToken.value = "-" + tokens[4].value;
    }

    if (variables.find(varName) != variables.end()) {
        std::cerr << "Warning: Variable already declared: " << varName
                  << ". Overwriting the existing value." << std::endl;
    }

    if (valueToken.type == STRING_LITERAL) {
        variables[varName] = { "string", valueToken.value };
    } else if (valueToken.type == NUMBER) {
        variables[varName] = { "number", valueToken.value };
    } else if (valueToken.value == "true" || valueToken.value == "false") {
        variables[varName] = { "boolean", valueToken.value };
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
        } else if (tokens[i].type == IDENTIFIER) {
            auto it = variables.find(tokens[i].value);
            if (it == variables.end()) {
                throw std::runtime_error("Undefined variable: " + tokens[i].value);
            }

            const std::string& varType = it->second.first;
            const std::string& varValue = it->second.second;

            if (varType == "string") {
                output << varValue;
            } else if (varType == "boolean") {
                output << (varValue == "true" ? "true" : "false");
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
                        case '/':
                            if (value == 0) throw std::runtime_error("Division by zero.");
                            mathResult /= value;
                            break;
                    }
                }
            }
        } else if (tokens[i].type == NUMBER) {
            double value = std::stod(tokens[i].value);
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
        } else {
            throw std::runtime_error("Unsupported token in send command.");
        }
    }

    if (mathStarted) {
        output << mathResult;
    }

    return output.str();
}

std::vector<Token> Interpreter::tokenize(const std::string& line) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < line.length()) {
        if (std::isspace(line[i])) {
            i++;
            continue;
        }

        if (line.substr(i, 4) == "send") {
            tokens.push_back({ SEND, "send" });
            i += 4;
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
        else if (line.substr(i, 4) == "help") {
            tokens.push_back({ HELP, "help" });
            i += 4;
            while (i < line.length() && std::isspace(line[i])) i++; // Skip spaces
            if (i < line.length()) {
                size_t start = i;
                while (i < line.length() && !std::isspace(line[i])) i++;
                tokens.push_back({ IDENTIFIER, line.substr(start, i - start) });
            }
        }
        else if (line[i] == '"') {
            size_t endQuote = line.find('"', i + 1);
            if (endQuote == std::string::npos) {
                throw std::runtime_error("Unterminated string literal");
            }
            tokens.push_back({ STRING_LITERAL, line.substr(i + 1, endQuote - i - 1) });
            i = endQuote + 1;
        }
        else if (line[i] == '-' && (i == 0 || tokens.back().type == OPERATOR || tokens.back().type == ASSIGNMENT)) {
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
        else if (line[i] == '=' || line[i] == '+' || line[i] == '-' || line[i] == '*' || line[i] == '/') {
            tokens.push_back({ line[i] == '=' ? ASSIGNMENT : OPERATOR, std::string(1, line[i]) });
            i++;
        } else {
            throw std::runtime_error("Invalid token at: " + std::string(1, line[i]));
        }
    }

    return tokens;
}