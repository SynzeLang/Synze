#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <chrono>

// Token Types
enum TokenType {
    NUMBER,
    IDENTIFIER,
    OPERATOR,
    ASSIGNMENT,
    SEND,
    STRING_LITERAL,
    RUN,
    VARIABLE,
    INVALID
};

// Token Structure
struct Token {
    TokenType type;
    std::string value;
};

// Interpreter Class
class Interpreter {
    std::map<std::string, std::pair<std::string, std::string>> variables; // Map variable name to <type, value>

public:
    void execute(const std::string& line) {
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
            throw std::runtime_error("Invalid command. Use 'send <expression>', 'variable <name> = <value>', or 'run <file>'.");
        }
    }

private:
    void handleVariableDeclaration(const std::vector<Token>& tokens) {
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

    void handleRunCommand(const std::string& filePath) {
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

        std::string handleSendCommand(const std::vector<Token>& tokens) {
        std::ostringstream output;
        std::string stringResult;
        double mathResult = 0;
        bool isMathMode = false;
        char currentOperator = '+';
        bool mathPerformed = false;

        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i].type == STRING_LITERAL) {
                if (isMathMode) {
                    if (mathPerformed || mathResult != 0) {
                        output << mathResult;
                    }
                    mathResult = 0;
                    isMathMode = false;
                }
                stringResult += tokens[i].value;
            } else if (tokens[i].type == IDENTIFIER) {
                auto it = variables.find(tokens[i].value);
                if (it != variables.end()) {
                    if (it->second.first == "string") {
                        if (isMathMode) {
                            if (mathPerformed || mathResult != 0) {
                                output << mathResult;
                            }
                            mathResult = 0;
                            isMathMode = false;
                        }
                        stringResult += it->second.second;
                    } else {
                        double value = getValueAsNumber(tokens[i]);
                        if (isMathMode) {
                            if (currentOperator == '+') mathResult += value;
                            else if (currentOperator == '-') mathResult -= value;
                            else if (currentOperator == '*') mathResult *= value;
                            else if (currentOperator == '/') {
                                if (value == 0) throw std::runtime_error("Division by zero.");
                                mathResult /= value;
                            }
                            mathPerformed = true;
                        } else {
                            mathResult = value;
                            mathPerformed = true;
                        }
                        isMathMode = true;
                    }
                } else {
                    throw std::runtime_error("Undefined variable: " + tokens[i].value);
                }
            } else if (tokens[i].type == NUMBER) {
                double value = getValueAsNumber(tokens[i]);
                if (isMathMode) {
                    if (currentOperator == '+') mathResult += value;
                    else if (currentOperator == '-') mathResult -= value;
                    else if (currentOperator == '*') mathResult *= value;
                    else if (currentOperator == '/') {
                        if (value == 0) throw std::runtime_error("Division by zero.");
                        mathResult /= value;
                    }
                    mathPerformed = true;
                } else {
                    mathResult = value;
                    mathPerformed = true;
                }
                isMathMode = true;
            } else if (tokens[i].type == OPERATOR) {
                if (tokens[i].value == "+" || tokens[i].value == "-" || tokens[i].value == "*" || tokens[i].value == "/") {
                    currentOperator = tokens[i].value[0];
                    isMathMode = true;
                } else {
                    throw std::runtime_error("Invalid operator in 'send' command.");
                }
            } else {
                throw std::runtime_error("Invalid token in 'send' command.");
            }
        }

        if (mathPerformed || mathResult != 0) {
            output << mathResult;
        }

        output << stringResult; // Append any accumulated string results

        return output.str();
    }

    double getValueAsNumber(const Token& token) {
        if (token.type == NUMBER) {
            return std::stod(token.value);
        } else if (token.type == IDENTIFIER) {
            auto it = variables.find(token.value);
            if (it != variables.end() && it->second.first == "number") {
                return std::stod(it->second.second);
            } else {
                throw std::runtime_error("Variable is not a number: " + token.value);
            }
        }
        throw std::runtime_error("Invalid token");
    }

        std::vector<Token> tokenize(const std::string& line) {
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
            } else if (line.substr(i, 3) == "run") {
                tokens.push_back({ RUN, "run" });
                i += 3;

                while (i < line.length() && std::isspace(line[i])) i++;
                size_t pathStart = i;
                while (i < line.length() && !std::isspace(line[i])) {
                    i++;
                }
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
            } else if (std::isdigit(line[i]) || (line[i] == '-' && i + 1 < line.length() && std::isdigit(line[i + 1]))) {
                size_t start = i;
                while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.')) i++;
                tokens.push_back({ NUMBER, line.substr(start, i - start) });
            } else if (std::isalpha(line[i]) || line[i] == '_' || line[i] == '/' || line[i] == ':' || line[i] == '.') {
                size_t start = i;
                while (i < line.length() && (std::isalnum(line[i]) || line[i] == '_' || line[i] == '/' || line[i] == ':' || line[i] == '.')) i++;
                tokens.push_back({ IDENTIFIER, line.substr(start, i - start) });
            } else if (line[i] == '=' || line[i] == '+' || line[i] == '-' || line[i] == '*' || line[i] == '/') {
                tokens.push_back({ line[i] == '=' ? ASSIGNMENT : OPERATOR, std::string(1, line[i]) });
                i++;
            } else {
                throw std::runtime_error("Invalid token");
            }
        }

        return tokens;
    }
};

int main() {
    Interpreter interpreter;
    std::cout << "\n#######  ##    ##  ###    ##  ########   #####   ##   ##  ##  ##    ##  ###    ### \n";
    std::cout << "##        ##  ##   ####   ##     ##     ##   ##   ## ##   ##  ##    ##  ####  #### \n";
    std::cout << "#######    ####    ## ##  ##     ##     #######    ###    ##  ##    ##  ## #### ## \n";
    std::cout << "     ##     ##     ##  ## ##     ##     ##   ##   ## ##   ##  ##    ##  ##  ##  ## \n";
    std::cout << "#######     ##     ##   ####     ##     ##   ##  ##   ##  ##   ######   ##      ## \n\n";
    std::cout << "The Syntaxium Interpreter is active.\nType 'exit' to quit.\n\n";

    std::string line;
    while (true) {
        try {
            std::cout << ">> ";
            std::getline(std::cin, line);
            if (line == "exit") break;
            interpreter.execute(line);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}