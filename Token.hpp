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
    COMMENT,
    FUNC,
    IF,
    ELSE_IF,
    ELSE,  
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
};

#endif