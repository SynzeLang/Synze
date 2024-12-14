#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TokenType {
    NUMBER,
    IDENTIFIER,
    OPERATOR,
    ASSIGNMENT,
    SEND,
    STRING_LITERAL,
    RUN,
    VARIABLE,
    EXIT,
    HELP,
    COMMENT,
    FUNC,       // Added for function keyword
    END,        // Added for the 'end' keyword or indentation tracking (if required)
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
};

#endif