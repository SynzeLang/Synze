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
    FUNCTION,
    COLON,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
};

#endif